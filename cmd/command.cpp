#include <ctime>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/command.hpp"
#include "ftp/client.hpp"
#include "fs/directory.hpp"
#include "fs/file.hpp"
#include "util/error.hpp"
#include "fs/status.hpp"
#include "fs/path.hpp"
#include "cmd/dirlist.hpp"
#include "cmd/factory.hpp"
#include "util/misc.hpp"
#include "util/net/ftp.hpp"
#include "acl/check.hpp"
#include "acl/usercache.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "ftp/data.hpp"
#include "main.hpp"

#include <iostream>
#include "logger/logger.hpp"

namespace PP = acl::PathPermission;


namespace cmd
{

void ABORCommand::Execute()
{
  control.Reply(ftp::DataClosedOkay, "ABOR command successful."); 
}

void ACCTCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "ACCT Command not implemented."); 
}

void ADATCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "ADAT Command not implemented."); 
}

void ALLOCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "ALLO Command not implemented."); 
}

void APPECommand::Execute()
{
  control.Reply(ftp::NotImplemented, "APPE Command not implemented."); 
}

void AUTHCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (argStr != "TLS" && argStr != "SSL")
  {
    control.Reply(ftp::ParameterNotImplemented,
                 "AUTH " + argStr + " is unsupported.");
    return;
  }
  
  control.Reply(ftp::SecurityExchangeOkay, "AUTH TLS successful."); 
  control.NegotiateTLS();  
}

void CCCCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "CCC Command not implemented."); 
}

void CDUPCommand::Execute()
{
  if (!argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  fs::Path path = "..";
  
  util::Error e = fs::ChangeDirectory(client,  path);
  if (!e) control.Reply(ftp::ActionNotOkay, "CDUP failed: " + e.Message());
  else control.Reply(ftp::FileActionOkay, "CDUP command successful."); 
}

void CONFCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "CONF Command not implemented."); 
}

void CWDCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  fs::Path path = argStr;
  
  util::Error e = fs::ChangeDirectory(client,  path);
  if (!e) control.Reply(ftp::ActionNotOkay, "CWD failed: " + e.Message());
  else if (path.ToString() != argStr)
    control.Reply(ftp::FileActionOkay, "CWD command successful (Matched: " + 
                 path.ToString() + ").");
  else
    control.Reply(ftp::FileActionOkay, "CWD command successful."); 
}

void DELECommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::DeleteFile(client,  argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, "DELE failed: " + e.Message());
  else control.Reply(ftp::FileActionOkay, "DELE command successful."); 
}

void ENCCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "ENC Command not implemented."); 
}

void EPRTCommand::Execute()
{
  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  util::net::Endpoint ep;
  util::Error e = util::net::ftp::EndpointFromEPRT(args[1], ep);
  if (!e)
  {
    control.Reply(ftp::SyntaxError, "EPRT failed: " + e.Message());
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
}

void EPSVCommand::Execute()
{
  if (!argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
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
                                 data.EPSVMode() == ftp::EPSVMode::Full);
  
  control.Reply(ftp::ExtendedPassiveMode, "Entering extended passive mode (" + 
               portString + ")");
}

void FEATCommand::Execute()
{
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
  control.Reply(ftp::SystemStatus, "End.");
}

void HELPCommand::Execute()
{
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
    
  control.MultiReply(ftp::HelpMessage, reply);
}

void LANGCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "LANG Command not implemented."); 
}

void LISTCommand::Execute()
{
  control.Reply(ftp::TransferStatusOkay,
               "Opening data connection for directory listing.");

  try
  {
    data.Open(ftp::TransferType::List);
  }
  catch (const util::net::NetworkError&e )
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return;
  }
  
  std::string options;
  std::string path;
  if (args.size() >= 2)
  {
    std::string::size_type optOffset = 0;
    if (args[1][0] == '-')
    {
      options = args[1].substr(1);
      optOffset += args[1].length();
    }
    
    path = std::string(argStr, optOffset);
    boost::trim(path);
  }

  
  const cfg::Config& config = cfg::Get();
  std::string forcedOptions = "l" + config.Lslong().Options();
  
  DirectoryList dirList(client, data, path, ListOptions(options, forcedOptions),
                        config.Lslong().MaxRecursion());

  try
  {
    dirList.Execute();    
  }
  catch (const util::net::NetworkError& e)
  {
    data.Close();
    control.Reply(ftp::DataCloseAborted,
                "Error whiling writing to data connection: " + e.Message());
    return;
  }
  
  data.Close();
  control.Reply(ftp::DataClosedOkay, "End of directory listing."); 
}

void LPRTCommand::Execute()
{
  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  util::net::Endpoint ep;
  util::Error e = util::net::ftp::EndpointFromLPRT(args[1], ep);
  if (!e)
  {
    control.Reply(ftp::SyntaxError, "LPRT failed: " + e.Message());
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
}

void LPSVCommand::Execute()
{
  if (!argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
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
}

void MDTMCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  fs::Path absolute = client.WorkDir() / args[1];
  
  util::Error e(PP::FileAllowed<PP::View>(client.User(), absolute));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, "MDTM failed: " + e.Message());
    return;
  }
  
  const std::string& Sitepath = cfg::Get().Sitepath();
  fs::Path real = fs::Path(Sitepath) + absolute;
  
  fs::Status status;
  try
  {
    status.Reset(real);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, "MDTM failed: " + e.Message());
    return;
  }
  
  char timestamp[15];
  strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S",
           localtime(&status.Native().st_mtime));
  control.Reply(ftp::FileStatus, timestamp);
}

void MICCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "MIC Command not implemented."); 
}

void MKDCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::CreateDirectory(client,  argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, "MKD failed: " + e.Message());
  else control.Reply(ftp::PathCreated, "MKD command successful."); 
}

void MLSDCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "MLSD Command not implemented."); 
}

void MLSTCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "MLST Command not implemented."); 
}

void MODECommand::Execute()
{
  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
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
}

void NLSTCommand::Execute()
{
  control.Reply(ftp::TransferStatusOkay,
               "Opening data connection for directory listing.");

  try
  {
    data.Open(ftp::TransferType::List);
  }
  catch (const util::net::NetworkError&e )
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return;
  }
  
  std::string options;
  std::string path;
  if (args.size() >= 2)
  {
    std::string::size_type optOffset = 0;
    if (args[1][0] == '-')
    {
      options = args[1].substr(1);
      optOffset += args[1].length();
    }
    
    path = argStr.substr(optOffset);
    boost::trim(path);
  }
  
  DirectoryList dirList(client, data, path, ListOptions(options, ""),
                        cfg::Get().Lslong().MaxRecursion());

  try
  {
    dirList.Execute();    
  }
  catch (const util::net::NetworkError& e)
  {
    control.Reply(ftp::DataCloseAborted,
                "Error whiling writing to data connection: " + e.Message());
  }
  
  data.Close();
  control.Reply(ftp::DataClosedOkay, "End of directory listing."); 
}

void NOOPCommand::Execute()
{
  control.Reply(ftp::CommandOkay, "NOOP command successful."); 
}

void OPTSCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "OPTS Command not implemented."); 
}

void PASSCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;    
  }
  
  if (!client.VerifyPassword(argStr))
  {
    if (client.PasswordAttemptsExceeded())
    {
      control.Reply(ftp::NotLoggedIn,
                  "Password attempts exceeded, disconnecting.");
      client.SetFinished();
    }
    else
    {
      control.Reply(ftp::NotLoggedIn, "Login incorrect.");
      client.SetLoggedOut();
    }
    return;
  }
  
  control.Reply(ftp::UserLoggedIn, "User " + client.User().Name() + " logged in.");
  client.SetLoggedIn();
}

void PASVCommand::Execute()
{
  if (!argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::net::Endpoint ep;
  try
  {
    data.InitPassive(ep, ftp::PassiveType::PASV);
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
    control.Reply(ftp::SyntaxError, "PASV failed: " + e.Message());
    return;
  }
  
  control.Reply(ftp::PassiveMode, "Entering passive mode (" + portString + ")");
}

void PBSZCommand::Execute()
{
  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] != "0")
    control.Reply(ftp::ParameterNotImplemented, "Only protection buffer size 0 supported.");
  else
    control.Reply(ftp::CommandOkay, "Protection buffer size set to 0.");
}

void PORTCommand::Execute()
{
  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

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
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return;
  }
  
  control.Reply(ftp::CommandOkay, "PORT command successful.");
}

void PROTCommand::Execute()
{
  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] == "P")
  {
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
}

void PWDCommand::Execute()
{
  control.Reply(ftp::PathCreated, "\"" + client.WorkDir().ToString() +
               "\" is your working directory.");
}

void QUITCommand::Execute()
{
  control.Reply(ftp::ClosingControl, "Bye bye"); 
  client.SetFinished();
}

void REINCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "REIN Command not implemented."); 
}

void RESTCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "REST Command not implemented."); 
}

void RETRCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
    
  fs::InStreamPtr fin;
  try
  {
    fin = fs::OpenFile(client, argStr);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay,
                 "Unable to open file: " + e.Message());
    return;
  }

  control.Reply(ftp::TransferStatusOkay,
               "Opening data connection for download of " + 
               fs::Path(argStr).Basename().ToString() + ".");

  try
  {
    data.Open(ftp::TransferType::Download);
  }
  catch (const util::net::NetworkError&e )
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " +
                 e.Message());
    return;
  }

  try
  {
    char buffer[16384];
    while (true)
    {
      ssize_t len = boost::iostreams::read(*fin,buffer, sizeof(buffer));
      if (len < 0) break;
      data.Write(buffer, len);
    }
  }
  catch (const std::ios_base::failure&)
  {
    fin->close();
    data.Close();
    control.Reply(ftp::DataCloseAborted,
                 "Error while reading from disk.");
    return;
  }
  catch (const util::net::NetworkError& e)
  {
    data.Close();
    control.Reply(ftp::DataCloseAborted,
                 "Error while writing to data connection: " +
                 e.Message());
    return;
  }
  
  data.Close();
  control.Reply(ftp::DataClosedOkay, "Transfer finished."); 
}

void RMDCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::RemoveDirectory(client,  argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, "RMD failed: " + e.Message());
  else control.Reply(ftp::FileActionOkay, "RMD command successful."); 
}

void RNFRCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  fs::Path absolute = (client.WorkDir() / argStr).Expand();
  
  util::Error e(PP::FileAllowed<PP::Rename>(client.User(), absolute));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, "RNFR failed: " + e.Message());
    return;
  }

  try
  {
    fs::Status status(absolute);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, "RNFR failed: " + e.Message());
    return;
  }
  
  control.PartReply(ftp::PendingMoreInfo, std::string(absolute));
  client.SetRenameFrom(absolute);
  control.Reply("File exists, ready for destination name."); 
}

void RNTOCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::RenameFile(client, client.RenameFrom(), argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, "RNTO failed: " + e.Message());
  else control.Reply(ftp::FileActionOkay, "RNTO command successful.");
}

void SITECommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  boost::to_upper(args[1]);
  if (args[1] == "EPSV")
  {
    static const char* syntax = "Syntax: SITE EPSV normal|full";
    if (args.size() == 2)
      control.Reply(ftp::CommandOkay, "Extended passive mode is currently '" +
                   std::string(data.EPSVMode() == ftp::EPSVMode::Normal ?
                   "normal" : "full") + "'.");
                    
    else if (args.size() != 3) control.Reply(ftp::SyntaxError, syntax);
    else
    {
      boost::to_upper(args[2]);
      if (args[2] == "NORMAL")
      {
        data.SetEPSVMode(ftp::EPSVMode::Normal);
        control.Reply(ftp::SyntaxError, "Extended passive mode now set to 'normal'.");
      }
      else if (args[2] == "FULL")
      {
        data.SetEPSVMode(ftp::EPSVMode::Full);
        control.Reply(ftp::SyntaxError, "Extended passive mode now set to 'full'.");
      }
      else
        control.Reply(ftp::SyntaxError, syntax);
    }
    return;
  }
  
  control.Reply(ftp::CommandUnrecognised, "SITE " + args[1] + 
               " command unrecognised."); 
}

void SIZECommand::Execute()
{
  fs::Path absolute = (client.WorkDir() / argStr).Expand();

  util::Error e(PP::FileAllowed<PP::View>(client.User(), absolute));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, "SIZE failed 2: " + e.Message());
    return;
  }
  
  fs::Status status;
  try
  {
    status.Reset(cfg::Get().Sitepath() + absolute);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, "SIZE failed 1: " + e.Message());
    return;
  }
  
  control.Reply(ftp::FileStatus, boost::lexical_cast<std::string>(status.Size())); 
}

void SMNTCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "SMNT Command not implemented."); 
}

void STATCommand::Execute()
{
  if (args.size() == 1)
  {
    control.PartReply(ftp::SystemStatus, programFullname + " status:");
    control.PartReply("< Insert status info here >");
    control.Reply("End of status.");
    return;
  }

  std::string options;
  std::string::size_type optOffset = 0;
  if (args[1][0] == '-')
  {
    options = args[1].substr(1);
    optOffset += args[1].length();
  }
  
  std::string path(argStr, optOffset);
  boost::trim(path);
  
  const cfg::Config& config = cfg::Get();
  std::string forcedOptions = "l" + config.Lslong().Options();
    
  control.PartReply(ftp::DirectoryStatus, "Status of " + path + ":");
  DirectoryList dirList(client, control, path, ListOptions(options, forcedOptions),
                        config.Lslong().MaxRecursion());
  dirList.Execute();
  
  control.Reply("End of status.");
}

void STORCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  fs::OutStreamPtr fout;
  try
  {
    fout = fs::CreateFile(client, argStr);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay,
                 "Unable to create file: " + e.Message());
    return;
  }

  control.Reply(ftp::TransferStatusOkay,
               "Opening data connection for upload of " +
               fs::Path(argStr).Basename().ToString() + ".");

  try
  {
    data.Open(ftp::TransferType::Upload);
  }
  catch (const util::net::NetworkError&e )
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return;
  }

  try
  {
    char buffer[16384];
    while (true)
    {
      size_t len = data.Read(buffer, sizeof(buffer));
      fout->write(buffer, len);
    }
  }
  catch (const util::net::EndOfStream&) { }
  catch (const util::net::NetworkError& e)
  {
    data.Close();
    control.Reply(ftp::DataCloseAborted,
                 "Error while reading from data connection: " +
                 e.Message());
    return;
  }
  
  data.Close();
  control.Reply(ftp::DataClosedOkay, "Transfer finished."); 
}

void STOUCommand::Execute()
{
  static size_t filenameLength = 10;

  if (!argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  fs::Path uniquePath;
  if (!fs::UniqueFile(client, client.WorkDir(),
                      filenameLength, uniquePath))
  {
    control.Reply(ftp::ActionNotOkay,
                 "Unable to generate a unique filename.");
    return;
  }
  
  argStr = uniquePath.ToString();
  args.clear();
  args.emplace_back("STOR");
  args.emplace_back(argStr);
  
  ftp::ClientState reqdState;

  std::unique_ptr<cmd::Command>
    command(cmd::Factory::Create(client, argStr, args, reqdState));
  assert(command.get());
  command->Execute();
}

void STRUCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "STRU Command not implemented."); 
}

void SYSTCommand::Execute()
{
  control.Reply(ftp::SystemType, "UNIX Type: L8"); 
}

void TYPECommand::Execute()
{
  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] != "I" && args[1] != "A")
  {
    control.Reply(ftp::ParameterNotImplemented,
                 "TYPE " + args[1] + " not supported.");
    return;
  }
  
  control.Reply(ftp::CommandOkay, "TYPE command successful."); 
}

void USERCommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  acl::User user;
  try
  {
    user = acl::UserCache::User(argStr);
  }
  catch (const util::RuntimeError& e)
  {
    if (argStr == "root")
      user = acl::User("root", 123, "password", "1");
    else
    {
      control.Reply(ftp::NotLoggedIn, "User " + argStr + " access denied.");
      return;
    }
  }

/*  if (argStr != client.User().Name())
  {
    control.Reply(ftp::NotLoggedIn, "User " + argStr + " access denied.");
    return;
  }*/
  
  control.Reply(ftp::NeedPassword, "Password required for " + argStr + "."); 
  client.SetWaitingPassword(user);
}

} /* cmd namespace */
