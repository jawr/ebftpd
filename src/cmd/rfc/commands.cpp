//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iomanip>
#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <utime.h>
#include <sys/types.h>
#include <sys/time.h>
#include "cmd/rfc/commands.hpp"
#include "cmd/rfc/factory.hpp"
#include "acl/flags.hpp"
#include "acl/misc.hpp"
#include "acl/path.hpp"
#include "acl/user.hpp"
#include "cfg/get.hpp"
#include "cmd/rfc/dirlist.hpp"
#include "cmd/error.hpp"
#include "cmd/site/factory.hpp"
#include "cmd/util.hpp"
#include "db/dupe/dupe.hpp"
#include "db/index/index.hpp"
#include "db/stats/stats.hpp"
#include "exec/check.hpp"
#include "exec/cscript.hpp"
#include "fs/directory.hpp"
#include "fs/file.hpp"
#include "fs/owner.hpp"
#include "fs/path.hpp"
#include "ftp/data.hpp"
#include "logs/logs.hpp"
#include "main.hpp"
#include "stats/util.hpp"
#include "text/util.hpp"
#include "util/net/ftp.hpp"
#include "util/net/tlscontext.hpp"
#include "util/path/status.hpp"
#include "util/scopeguard.hpp"
#include "util/string.hpp"

namespace cmd { namespace rfc
{

void ABORCommand::Execute()
{
  control.Reply(ftp::DataClosedOkay, "ABOR command successful."); 
  return;
}

void AUTHCommand::Execute()
{
  if (!util::net::TLSServerContext::Get())
  {
    control.Reply(ftp::ParameterNotImplemented, "TLS is not enabled.");
    return;
  }
  
  if (args[1] == "SSL") data.SetProtection(true);
  else if (args[1] != "TLS")
  {
    control.Reply(ftp::ParameterNotImplemented,
                 "AUTH " + args[1] + " is unsupported.");
    return;
  }
  
  control.Reply(ftp::SecurityExchangeOkay, "AUTH " + args[1] + " successful."); 
  control.NegotiateTLS();
}

void CDUPCommand::Execute()
{
  util::Error e = fs::ChangeDirectory(client.User(), 
        fs::Resolve(fs::MakeVirtual(fs::Path(".."))));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, "..: " + e.Message());
    throw cmd::NoPostScriptError();
  }
  
  control.Reply(ftp::FileActionOkay, "CDUP command successful."); 
}

void CPSVCommand::Execute()
{
  util::net::Endpoint ep;
  try
  {
    data.InitPassive(ep, ftp::PassiveType::CPSV);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection: " + e.Message());
    return;
  }

  std::string portString;
  util::Error e = util::net::ftp::EndpointToPORT(ep, portString);
  if (!e)
  {
    control.Reply(ftp::SyntaxError, e.Message());
    return;
  }
  
  control.Reply(ftp::PassiveMode, "Entering passive mode (" + portString + ")");
}

void DELECommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));

  bool loseCredits = fs::GetOwner(fs::MakeReal(path)).UID() == client.User().ID();
  
  off_t bytes;
  time_t modTime;
  util::Error e = fs::DeleteFile(client.User(),  path, &bytes, &modTime);
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    throw cmd::NoPostScriptError();
  }
  
  auto section = cfg::Get().SectionMatch(path.ToString());
  bool nostats = !section || acl::path::FileAllowed<acl::path::Nostats>(client.User(), path);
  if (!nostats)
  {
    db::stats::UploadDecr(client.User(), bytes / 1024, modTime, section->Name());
  }

  if (loseCredits)
  {
    long long creditLoss = bytes / 1024 * stats::UploadRatio(client.User(), path, section);
    if (creditLoss)
    {
      client.User().DecrSectionCredits(section && section->SeparateCredits() ? section->Name() : "", creditLoss);
      std::ostringstream os;
      os << "DELE command successful. (" << util::ToString(creditLoss / 1024.0, 2) << "MB credits lost)";
      control.Reply(ftp::FileActionOkay, os.str()); 
      return;
    }
  }

  control.Reply(ftp::FileActionOkay, "DELE command successful."); 
}

void EPRTCommand::Execute()
{
  util::net::Endpoint ep;
  util::Error e = util::net::ftp::EndpointFromEPRT(args[1], ep);
  if (!e)
  {
    control.Reply(ftp::SyntaxError, e.Message());
    return;
  }
  
  try
  {
    data.InitActive(ep);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return;
  }
  
  control.Reply(ftp::CommandOkay, "EPRT command successful.");
  return;
}

void EPSVCommand::ChangeMode()
{
  auto epsvFxp = cfg::Get().EPSVFxp();
  util::ToUpper(args[1]);
  if (args[1] == "EXTENDED") 
  {
    if (epsvFxp == ::cfg::EPSVFxp::Deny)
    {
      control.Reply(ftp::ParameterNotImplemented, "EPSV EXTENDED not supported.");
      return;
    }
    client.Data().SetEPSVMode(ftp::EPSVMode::Extended);
  }
  else if (args[1] == "NORMAL") 
  {
    if (epsvFxp == ::cfg::EPSVFxp::Force)
    {
      control.Reply(ftp::ParameterNotImplemented, "EPSV NORMAL not supported.");
      return;
    }
    client.Data().SetEPSVMode(ftp::EPSVMode::Normal);
  }
  else if (args[1] != "MODE")
    throw cmd::SyntaxError();
  
  std::ostringstream os;
  os << util::EnumToString(epsvFxp) << " (" 
     << util::EnumToString(client.Data().EPSVMode()) << ").";
  control.Reply(ftp::CommandOkay, os.str());
}

void EPSVCommand::Execute()
{
  if (args.size() == 2)
  {
    ChangeMode();
    return;
  }

  util::net::Endpoint ep;
  try
  {
    data.InitPassive(ep, ftp::PassiveType::EPSV);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection: " + e.Message());
    return;
  }

  std::string portString;
  util::net::ftp::EndpointToEPRT(ep, portString, 
                                 data.EPSVMode() == ftp::EPSVMode::Extended);
  
  control.Reply(ftp::ExtendedPassiveMode, "Entering extended passive mode (" + 
               portString + ")");

  return;
}

void FEATCommand::Execute()
{
  bool singleLineReplies = control.SingleLineReplies();
  control.SetSingleLineReplies(false);
  
  auto singleLineGuard = util::MakeScopeExit([&]{ control.SetSingleLineReplies(singleLineReplies); });  

  control.PartReply(ftp::SystemStatus, "Extended feature support:");
  control.PartReply(ftp::NoCode, " AUTH TLS");
  control.PartReply(ftp::NoCode, " EPRT");
  control.PartReply(ftp::NoCode, " EPSV");
  control.PartReply(ftp::NoCode, " LPRT");
  control.PartReply(ftp::NoCode, " LPSV");
  control.PartReply(ftp::NoCode, " PBSZ");
  control.PartReply(ftp::NoCode, " PROT");
  control.PartReply(ftp::NoCode, " MDTM");
  control.PartReply(ftp::NoCode, " SIZE");
  control.PartReply(ftp::NoCode, " SSCN");
  control.PartReply(ftp::NoCode, " CPSV");
  control.PartReply(ftp::NoCode, " MFMT");
  control.Reply(ftp::SystemStatus, "End.");

  (void) singleLineReplies;
  (void) singleLineGuard;
}

void HELPCommand::Execute()
{
  if (args.size() == 2)
  {
    util::ToUpper(args[1]);
    CommandDefOptRef def(Factory::Lookup(args[1]));
    if (!def) control.Reply(ftp::CommandUnrecognised, "Command not understood");
    else control.Reply(ftp::CommandOkay, "Syntax: " + def->Syntax());
    return;
  }

  static const char* reply =
    " ebftpd Command listing:\n"
    "------------------------------------------------------------------\n"
    " ABOR *ACCT *ADAT *ALLO  APPE  AUTH *CCC   CDUP *CONF  CWD   DELE\n"
    "*ENC   EPRT  EPSV  FEAT  HELP *LANG  LIST *LPRT *LPSV  MDTM *MIC\n"
    " MKD  *MLSD *MLST  MODE  NLST  NOOP *OPTS  PASS  PASV  PBSZ  PORT\n"
    " PROT  PWD   QUIT *REIN *REST  RETR  RMD   RNFR  RNTO  SITE  SIZE\n"
    "*SMNT  STAT  STOR  STOU *STRU  SYST  TYPE\n"
    "------------------------------------------------------------------\n"
    "End of list.                         (* Commands not implemented)";
    
  control.Reply(ftp::HelpMessage, reply);
  return;
}


void LPRTCommand::Execute()
{
  util::net::Endpoint ep;
  util::Error e = util::net::ftp::EndpointFromLPRT(args[1], ep);
  if (!e)
  {
    control.Reply(ftp::SyntaxError, args[1] + ": " + e.Message());
    return;
  }
  
  try
  {
    data.InitActive(ep);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return;
  }
  
  control.Reply(ftp::CommandOkay, "LPRT command successful.");
  return;
}

void LPSVCommand::Execute()
{
  util::net::Endpoint ep;
  try
  {
    data.InitPassive(ep, ftp::PassiveType::LPSV);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection: " + e.Message());
    return;
  }
  
  std::string portString;
  util::net::ftp::EndpointToLPRT(ep, portString);
  
  control.Reply(ftp::LongPassiveMode, "Entering passive mode (" + portString + ")");
  return;
}

void MFFCommand::Execute()
{
}

void MFCTCommand::Execute()
{
}

void MFMTCommand::Execute()
{
  std::string pathStr(argStr.substr(args[1].length() + 1));
  util::Trim(pathStr);
  fs::VirtualPath path(fs::PathFromUser(pathStr));

  struct tm tm;
  if (!strptime(args[1].c_str(), "%Y%m%d%H%M%S", &tm)) throw cmd::SyntaxError();
  
  time_t t = timegm(&tm);
  struct timeval tv[2] =  { { t, 0 }, { t, 0 } };

  util::Error e(acl::path::FileAllowed<acl::path::Modify>(client.User(), path));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, pathStr + ": " + e.Message());
    return;
  }
  
  try
  {
    if (!util::path::Status(fs::MakeReal(path).ToString()).IsRegularFile())
    {
      control.Reply(ftp::ActionNotOkay, pathStr + ": Not a plain file.");
      return;
    }
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, pathStr + ": " + e.Message());
    return;
  }
  
  auto real(fs::MakeReal(path));
  if (utimes(real.CString(), tv))
  {
    control.Reply(ftp::ActionNotOkay, pathStr + ": " + util::Error::Failure(errno).Message());
    return;
  }
  
  control.Reply(ftp::FileStatus, "Modify=" + args[1] + "; " + pathStr);
}

void MDTMCommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));
  
  util::Error e(acl::path::FileAllowed<acl::path::View>(client.User(), path));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return;
  }
  
  util::path::Status status;
  try
  {
    status.Reset(fs::MakeReal(path).ToString());
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return;
  }

  if (!status.IsRegularFile())
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": Not a plain file.");
    return;
  }
  
  char timestamp[15];
  strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S",
           gmtime(&status.Native().st_mtime));
  control.Reply(ftp::FileStatus, timestamp);
}

void MKDCommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));

  util::Error e(acl::path::Filter(client.User(), path.Basename()));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, "Directory name contains one or more invalid characters.");
    throw cmd::NoPostScriptError();
  }
  
  if (!exec::PreDirCheck(client, path)) throw cmd::NoPostScriptError();

  e = fs::CreateDirectory(client.User(),  path);
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    throw cmd::NoPostScriptError();
  }
  
  const cfg::Config& config = cfg::Get();
  
  bool indexed = config.IsIndexed(path.ToString());
  bool dupeLogged = config.IsDupeLogged(path.ToString());
  if (indexed | dupeLogged)
  {
    auto section = config.SectionMatch(path.ToString(), true);
    if (indexed) db::index::Add(path.ToString(), section ? section->Name() : "");
    if (dupeLogged) db::dupe::Add(path.Basename().ToString(), section ? section->Name() : "");    
  }
  
  if (config.IsEventLogged(path.ToString()))
  {
    logs::Event("NEWDIR", "path", fs::MakeReal(path).ToString(),
                "user", client.User().Name(), 
                "group", client.User().PrimaryGroup(), 
                "tagline", client.User().Tagline());
  }
  
  control.Reply(ftp::PathCreated, "MKD command successful.");  
}



void MODECommand::Execute()
{
  if (args[1] == "S")
    control.Reply(ftp::CommandOkay, "Transfer mode set to 'stream'.");
  else if (args[1] == "B")
    control.Reply(ftp::ParameterNotImplemented,
                 "Transfer mode 'block' not implemented.");
  else if (args[1] == "C")
    control.Reply(ftp::ParameterNotImplemented,
                 "Transfer mode 'compressed' not implemented.");
  else
    control.Reply(ftp::SyntaxError, "Unrecognised transfer mode.");
  return;
}







void NOOPCommand::Execute()
{
  control.Reply(ftp::CommandOkay, "NOOP command successful."); 
  return;
}

void PASVCommand::Execute()
{
  util::net::Endpoint ep;
  try
  {
    data.InitPassive(ep, ftp::PassiveType::PASV);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection: " + e.Message());
    throw cmd::NoPostScriptError();
  }

  std::string portString;
  util::Error e = util::net::ftp::EndpointToPORT(ep, portString);
  if (!e)
  {
    control.Reply(ftp::SyntaxError, e.Message());
    throw cmd::NoPostScriptError();
  }
  
  control.Reply(ftp::PassiveMode, "Entering passive mode (" + portString + ")");
}



void PBSZCommand::Execute()
{
  if (args[1] != "0")
    control.Reply(ftp::ParameterNotImplemented, "Only protection buffer size 0 supported.");
  else
    control.Reply(ftp::CommandOkay, "Protection buffer size set to 0.");
  return;
}

void PORTCommand::Execute()
{
  util::net::Endpoint ep;
  util::Error e = util::net::ftp::EndpointFromPORT(args[1], ep);
  if (!e)
  {
    control.Reply(ftp::SyntaxError, "Invalid port string.");
    return;
  }
  
  try
  {
    data.InitActive(ep);
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::CantOpenDataConnection, "Unable to open data connection: " + e.Message());
    return;
  }
  
  control.Reply(ftp::CommandOkay, "PORT command successful.");
  return;
}

void PROTCommand::Execute()
{
  if (args[1] == "P")
  {
    if (!util::net::TLSServerContext::Get() ||
        !util::net::TLSClientContext::Get())
    {
      control.Reply(ftp::ParameterNotImplemented,
                    "TLS is not enabled.");
      return;
    }
    
    data.SetProtection(true);
    control.Reply(ftp::CommandOkay, "Protection type set to 'private'.");
  }
  else if (args[1] == "C")
  {
    data.SetProtection(false);
    control.Reply(ftp::CommandOkay, "Protection type set to 'clear'.");
  }
  else if (args[1] == "S")
    control.Reply(ftp::ParameterNotImplemented,
                 "Protection type 'secure' not implemented.");
  else if (args[1] == "E")
    control.Reply(ftp::ParameterNotImplemented,
                 "Protection type 'confidential' not implemented.");
  else
    control.Reply(ftp::SyntaxError, "Unrecognised protection type.");
  return;
}

void PWDCommand::Execute()
{
  control.Reply(ftp::PathCreated, "\"" + fs::WorkDirectory().ToString() +
               "\" is your working directory.");
}

void QUITCommand::Execute()
{
  std::string goodbye;
  if (client.State() == ftp::ClientState::LoggedIn)
  {
    fs::Path goodbyePath(acl::message::Choose<acl::message::Goodbye>(client.User()));
    if (!goodbyePath.IsEmpty() &&
        text::GenericTemplate(client, goodbyePath, goodbye))
    {
      control.Reply(ftp::ClosingControl, goodbye);
    }
  }
  
  if (goodbye.empty()) control.Reply(ftp::ClosingControl, "Bye bye");

  client.SetState(ftp::ClientState::Finished);
}

void RESTCommand::Execute()
{
  off_t restart;
  
  try
  {
    restart = util::StrToInt(args[1]);
    if (restart < 0) throw std::bad_cast();
  }
  catch (const std::bad_cast&)
  {
    control.Reply(ftp::InvalidRESTParameter, "Invalid parameter, restart offset set to 0.");
    data.SetRestartOffset(0);
    return;
  }
  
  data.SetRestartOffset(restart);
  
  std::ostringstream os;
  os << "Restart offset set to " << restart << ".";
  control.Reply(ftp::CommandOkay, os.str()); 
}

void RMDCommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));
  util::Error e = fs::RemoveDirectory(client.User(),  path);
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    throw cmd::NoPostScriptError();
  }

  const cfg::Config& config = cfg::Get();
  
  if (config.IsIndexed(path.ToString()))
    db::index::Delete(path.ToString());
  
  if (config.IsEventLogged(path.ToString()))
  {
    logs::Event("DELDIR", "path", fs::MakeReal(path).ToString(), "user", client.User().Name(), 
                "group", client.User().PrimaryGroup(),
                "tagline", client.User().Tagline());
  }

  control.Reply(ftp::FileActionOkay, "RMD command successful.");   
}

void RNFRCommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));
  
  util::Error e(acl::path::Allowed<acl::path::Rename>(client.User(), path));
  if (!e)
  {
    if (e.Errno() != EACCES  ||
        !acl::path::Allowed<acl::path::Move>(client.User(), path))
    {
      control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
      throw cmd::NoPostScriptError();
    }
  }
  
  client.SetRenameFrom(std::make_pair(path, argStr));
  control.Reply(ftp::PendingMoreInfo, "File exists, ready for destination name."); 
}

void RNTOCommand::Execute()
{
  namespace PP = acl::path;

  if (!client.RenameFrom())
  {
    control.Reply(ftp::BadCommandSequence, "Invalid command sequence.");
    throw cmd::NoPostScriptError();
  }
  
  auto fromArgStr = client.RenameFrom()->second;
  auto oldPath = client.RenameFrom()->first;
  client.SetRenameFrom(boost::none);

  fs::VirtualPath newPath(fs::PathFromUser(argStr));

  util::Error e(acl::path::Filter(client.User(), newPath.Basename()));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, "Path name contains one or more invalid characters.");
    throw cmd::NoPostScriptError();
  }
  
  if (oldPath.Dirname() != newPath.Dirname()) // this is move
  {
    e = PP::Allowed<PP::Move>(client.User(), oldPath);
    if (!e)
    {
      control.Reply(ftp::ActionNotOkay, fromArgStr + ": " + e.Message());
      throw cmd::NoPostScriptError();
    }
  }
  
  if (oldPath.Basename() != newPath.Basename()) // this is rename
  {
    e = PP::Allowed<PP::Rename>(client.User(), oldPath);
    if (!e)
    {
      control.Reply(ftp::ActionNotOkay, fromArgStr + ": " + e.Message());
      throw cmd::NoPostScriptError();
    }
  }
  
  try
  {
    bool isDirectory = util::path::Status(fs::MakeReal(oldPath).ToString()).IsDirectory();
    if (isDirectory)
    {
      e = PP::DirAllowed<PP::Makedir>(client.User(), newPath);
    }
    else
    {
      e = PP::FileAllowed<PP::Upload>(client.User(), newPath);
    }
    
    if (!e)
    {
      control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
      throw cmd::NoPostScriptError();
    }
    
    e = fs::Rename(fs::MakeReal(oldPath), fs::MakeReal(newPath));    
    if (!e)
    {
      control.Reply(ftp::ActionNotOkay, fromArgStr + " -> " + argStr + ": " + e.Message());
      throw cmd::NoPostScriptError();
    }

    if (isDirectory)
    {
      // this should be changed to a single move action so as to retain the
      // creation date in the database
      if (cfg::Get().IsIndexed(oldPath.ToString()))
      {
        db::index::Delete(oldPath.ToString());
      }

      if (cfg::Get().IsIndexed(newPath.ToString()))
      {
        auto section = cfg::Get().SectionMatch(newPath.ToString(), true);
        db::index::Add(newPath.ToString(), section ? section->Name() : "");
      }
    }
    
    control.Reply(ftp::FileActionOkay, "RNTO command successful.");
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, fromArgStr + ": " + e.Message());
    throw cmd::NoPostScriptError();
  }
}



void SITECommand::Execute()
{
  cmd::SplitArgs(argStr, args);
  util::ToUpper(args[0]);
  argStr = argStr.substr(args[0].length());
  util::Trim(argStr);

  std::string shortCommand = "SITE " + args[0];
  std::string fullCommand = shortCommand;
  if (!argStr.empty()) fullCommand += " " + argStr;

  cmd::site::CommandDefOpt def(cmd::site::Factory::Lookup(args[0]));
  if (!def)
  { 
    control.Reply(ftp::CommandUnrecognised, "Command not understood");
  }
  else if (!acl::AllowSiteCmd(client.User(), def->ACLKeyword()))
  {
    control.Reply(ftp::ActionNotOkay,  "SITE " + args[0] + ": Permission denied");
    logs::Security("COMMANDACL", "'%1%' attempted to run command without permission: %2%",
                   client.User().Name(), fullCommand);
  }
  else if (!def->CheckArgs(args))
  {
    control.Reply(ftp::SyntaxError, def->Syntax());
  }
  else if (exec::Cscripts(client, shortCommand, fullCommand, exec::CscriptType::Pre,
              ftp::ActionNotOkay))
  {
    cmd::CommandPtr command(def->Create(client, argStr, args));
    if (!command)
    {
      control.Reply(ftp::NotImplemented, "Command not implemented");
    }
    else
    {
      if (!util::IsASCIIOnly(argStr))
      {
        control.Reply(ftp::SyntaxError, "SITE command arguments must contain ASCII characters only");
        return;
      }
    
      try
      {
        command->Execute();
        exec::Cscripts(client, shortCommand, fullCommand, exec::CscriptType::Post, 
                ftp::ActionNotOkay);
      }
      catch (const cmd::SyntaxError&)
      {
        control.Reply(ftp::SyntaxError, def->Syntax());
      }
      catch (const cmd::PermissionError&)
      {
        control.Reply(ftp::ActionNotOkay, "SITE " + args[0] + ": Permission denied");
        logs::Security("COMMANDACL", "'%1%' attempted to run command without permission: %2%",
                       client.User().Name(), fullCommand);
      }
    }
  }
}



void SIZECommand::Execute()
{
  namespace PP = acl::path;

  fs::VirtualPath path(fs::PathFromUser(argStr));

  try
  {
    auto e = acl::path::FileAllowed<acl::path::View>(client.User(), path);
    if (!e) throw util::SystemError(e.Errno());
      
    util::path::Status status(fs::MakeReal(path).ToString());
    if (status.IsRegularFile())
      control.Reply(ftp::FileStatus, std::to_string(status.Size())); 
    else
      control.Reply(ftp::ActionNotOkay, argStr + ": Not a plain file.");
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return;
  }
  
  return;
}



void SSCNCommand::Execute()
{
  if (args.size() == 2)
  {
    util::ToLower(args[1]);
    if (args[1] == "on") data.SetSSCNMode(ftp::SSCNMode::Client);
    else
    if (args[1] == "off") data.SetSSCNMode(ftp::SSCNMode::Server);
    else
      throw cmd::SyntaxError();
  }

  std::stringstream os;
  os << "SSCN:";
  if (data.SSCNMode() == ftp::SSCNMode::Server) os << "SERVER METHOD";
  else os << "CLIENT METHOD";
  control.Reply(ftp::CommandOkay, os.str());  
}

void STOUCommand::Execute()
{
  static size_t filenameLength = 10;

  fs::VirtualPath uniquePath;
  if (!fs::UniqueFile(client.User(), fs::WorkDirectory(),
                      filenameLength, uniquePath))
  {
    control.Reply(ftp::ActionNotOkay,
                 "Unable to generate a unique filename.");
    throw cmd::NoPostScriptError();
  }
  
  args.clear();
  args.emplace_back("STOR");
  args.emplace_back(uniquePath.ToString());  

  CommandPtr command(cmd::rfc::Factory::Lookup("STOR")->Create(client, uniquePath.ToString(), args));
  assert(command.get());
  command->Execute();
}



void SYSTCommand::Execute()
{
  control.Reply(ftp::SystemType, "UNIX Type: L8"); 
  return;
}



void TYPECommand::Execute()
{
  if (args[1] != "I" && args[1] != "A")
  {
    control.Reply(ftp::ParameterNotImplemented,
                 "TYPE " + args[1] + " not supported.");
    throw cmd::NoPostScriptError();
  }
  
  data.SetDataType(args[1] == "I" ? ftp::DataType::Binary : ftp::DataType::ASCII);
  
  control.Reply(ftp::CommandOkay, "TYPE command successful."); 
}



void USERCommand::Execute()
{
  bool kickLogin = false;
  if (argStr[0] == '!')
  {
    argStr.erase(0, 1);
    kickLogin = true;
  }
  
  try
  {
    auto user = acl::User::Load(argStr);
    if (!user || user->HasFlag(acl::Flag::Template))
    {
      logs::Security("LOGINUNKNOWN", "Unknown user '%1%' attempted to login", argStr);
      control.Reply(ftp::NotLoggedIn, "User " + argStr + " access denied.");
      return;
    }

    if (cfg::Get().TLSControl().Evaluate(user->ACLInfo()) && !control.IsTLS())
    {
      logs::Security("TLSCONTROL", "'%1%' attempted to login without TLS enabled on control", user->Name());
      control.Reply(ftp::NotLoggedIn, "TLS is enforced on control connections.");
      return;
    }
    
    client.SetWaitingPassword(*user, kickLogin);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::NotLoggedIn, e.Message());
    return;
  }
  
  control.Reply(ftp::NeedPassword, "Password required for " + argStr + "."); 
}

} /* rfc namespace */
} /* cmd namespace */
