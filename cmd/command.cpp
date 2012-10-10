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
#include <iostream>
#include "logger/logger.hpp"

namespace PP = acl::PathPermission;


namespace cmd
{

void ABORCommand::Execute()
{
  client.Reply(ftp::DataClosedOkay, "ABOR command successful."); 
}

void ACCTCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "ACCT Command not implemented."); 
}

void ADATCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "ADAT Command not implemented."); 
}

void ALLOCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "ALLO Command not implemented."); 
}

void APPECommand::Execute()
{
  client.Reply(ftp::NotImplemented, "APPE Command not implemented."); 
}

void AUTHCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (argStr != "TLS")
  {
    client.Reply(ftp::ParameterNotImplemented,
                 "AUTH " + argStr + " is unsupported.");
    return;
  }
  
  client.Reply(ftp::SecurityExchangeOkay, "AUTH TLS successful."); 
  client.NegotiateTLS();  
}

void CCCCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "CCC Command not implemented."); 
}

void CDUPCommand::Execute()
{
  if (!argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  fs::Path path = "..";
  
  util::Error e = fs::ChangeDirectory(client,  path);
  if (!e) client.Reply(ftp::ActionNotOkay, "CDUP failed: " + e.Message());
  else client.Reply(ftp::FileActionOkay, "CDUP command successful."); 
}

void CONFCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "CONF Command not implemented."); 
}

void CWDCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  fs::Path path = argStr;
  
  util::Error e = fs::ChangeDirectory(client,  path);
  if (!e) client.Reply(ftp::ActionNotOkay, "CWD failed: " + e.Message());
  else if (path.ToString() != argStr)
    client.Reply(ftp::FileActionOkay, "CWD command successful (Matched: " + 
                 path.ToString() + ").");
  else
    client.Reply(ftp::FileActionOkay, "CWD command successful."); 
}

void DELECommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::DeleteFile(client,  argStr);
  if (!e) client.Reply(ftp::ActionNotOkay, "DELE failed: " + e.Message());
  else client.Reply(ftp::FileActionOkay, "DELE command successful."); 
}

void ENCCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "ENC Command not implemented."); 
}

void EPRTCommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  util::net::Endpoint ep;
  util::Error e = util::net::ftp::EndpointFromEPRT(args[1], ep);
  if (!e)
  {
    client.Reply(ftp::SyntaxError, "EPRT failed: " + e.Message());
    return;
  }
  
  try
  {
    client.DataInitActive(ep);
  }
  catch (const util::net::NetworkError& e)
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return;
  }
  
  client.Reply(ftp::CommandOkay, "EPRT command successful.");
}

void EPSVCommand::Execute()
{
  if (!argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::net::Endpoint ep;
  try
  {
    client.DataInitPassive(ep, ftp::PassiveType::EPSV);
  }
  catch (const util::net::NetworkError& e)
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection: " + e.Message());
    return;
  }

  std::string portString;
  util::net::ftp::EndpointToEPRT(ep, portString, 
                                 client.EPSVMode() == ftp::EPSVMode::Full);
  
  client.Reply(ftp::ExtendedPassiveMode, "Entering extended passive mode (" + 
               portString + ")");
}

void FEATCommand::Execute()
{
  client.PartReply(ftp::SystemStatus, "Extended feature support:");
  client.PartReply(ftp::NoCode, " AUTH TLS");
  client.PartReply(ftp::NoCode, " EPRT");
  client.PartReply(ftp::NoCode, " EPSV");
  client.PartReply(ftp::NoCode, " LPRT");
  client.PartReply(ftp::NoCode, " LPSV");
  client.PartReply(ftp::NoCode, " PBSZ");
  client.PartReply(ftp::NoCode, " PROT");
  client.PartReply(ftp::NoCode, " MDTM");
  client.PartReply(ftp::NoCode, " SIZE");
  client.PartReply(ftp::NoCode, " SIZE");
  client.Reply(ftp::SystemStatus, "End.");
}

void HELPCommand::Execute()
{
  static const char* reply =
    "FTP Command listing:\n"
    "------------------------------------------------------------------\n"
    " ABOR *ACCT *ADAT *ALLO  APPE  AUTH *CCC   CDUP *CONF  CWD   DELE\n"
    "*ENC   EPRT  EPSV  FEAT  HELP *LANG  LIST *LPRT *LPSV  MDTM *MIC\n"
    " MKD  *MLSD *MLST  MODE  NLST  NOOP *OPTS  PASS  PASV  PBSZ  PORT\n"
    " PROT  PWD   QUIT *REIN *REST  RETR  RMD   RNFR  RNTO  SITE  SIZE\n"
    "*SMNT  STAT  STOR  STOU *STRU  SYST  TYPE\n"
    "------------------------------------------------------------------\n"
    "End of list.                         (* Commands not implemented)";
    
  client.MultiReply(ftp::HelpMessage, reply);
}

void LANGCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "LANG Command not implemented."); 
}

void LISTCommand::Execute()
{
  client.Reply(ftp::TransferStatusOkay,
               "Opening data connection for directory listing.");

  try
  {
    client.DataOpen(ftp::TransferType::List);
  }
  catch (const util::net::NetworkError&e )
  {
    client.Reply(ftp::CantOpenDataConnection,
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
    logger::ftpd << "path: " << path << logger::endl;
  }

  logger::ftpd << "Options: " << options << " Path: " << path << " Arg.size(): " << args.size() << " ArgStr: " << argStr << " args[0]: " << args.at(0) << logger::endl;
  
  const cfg::Config& config = cfg::Get();
  std::string forcedOptions = "l" + config.Lslong().Options();
  
  DirectoryList dirList(client, path, ListOptions(options, forcedOptions),
                        true, config.Lslong().MaxRecursion());

  try
  {
    dirList.Execute();    
  }
  catch (const util::net::NetworkError& e)
  {
    client.DataClose();
    client.Reply(ftp::DataCloseAborted,
                "Error whiling writing to data connection: " + e.Message());
    return;
  }
  
  client.DataClose();
  client.Reply(ftp::DataClosedOkay, "End of directory listing."); 
}

void LPRTCommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  util::net::Endpoint ep;
  util::Error e = util::net::ftp::EndpointFromLPRT(args[1], ep);
  if (!e)
  {
    client.Reply(ftp::SyntaxError, "LPRT failed: " + e.Message());
    return;
  }
  
  try
  {
    client.DataInitActive(ep);
  }
  catch (const util::net::NetworkError& e)
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return;
  }
  
  client.Reply(ftp::CommandOkay, "LPRT command successful.");
}

void LPSVCommand::Execute()
{
  if (!argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::net::Endpoint ep;
  try
  {
    client.DataInitPassive(ep, ftp::PassiveType::LPSV);
  }
  catch (const util::net::NetworkError& e)
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection: " + e.Message());
    return;
  }
  
  std::string portString;
  util::net::ftp::EndpointToLPRT(ep, portString);
  
  client.Reply(ftp::LongPassiveMode, "Entering passive mode (" + portString + ")");
}

void MDTMCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  fs::Path absolute = client.WorkDir() / args[1];
  
  util::Error e(PP::FileAllowed<PP::View>(client.User(), absolute));
  if (!e)
  {
    client.Reply(ftp::ActionNotOkay, "MDTM failed: " + e.Message());
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
    client.Reply(ftp::ActionNotOkay, "MDTM failed: " + e.Message());
    return;
  }
  
  char timestamp[15];
  strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S",
           localtime(&status.Native().st_mtime));
  client.Reply(ftp::FileStatus, timestamp);
}

void MICCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "MIC Command not implemented."); 
}

void MKDCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::CreateDirectory(client,  argStr);
  if (!e) client.Reply(ftp::ActionNotOkay, "MKD failed: " + e.Message());
  else client.Reply(ftp::PathCreated, "MKD command successful."); 
}

void MLSDCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "MLSD Command not implemented."); 
}

void MLSTCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "MLST Command not implemented."); 
}

void MODECommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] == "S")
    client.Reply(ftp::CommandOkay, "Transfer mode set to 'stream'.");
  else if (args[1] == "B")
    client.Reply(ftp::ParameterNotImplemented,
                 "Transfer mode 'block' not implemented.");
  else if (args[1] == "C")
    client.Reply(ftp::ParameterNotImplemented,
                 "Transfer mode 'compressed' not implemented.");
  else
    client.Reply(ftp::SyntaxError, "Unrecognised transfer mode.");
}

void NLSTCommand::Execute()
{
  client.Reply(ftp::TransferStatusOkay,
               "Opening data connection for directory listing.");

  try
  {
    client.DataOpen(ftp::TransferType::List);
  }
  catch (const util::net::NetworkError&e )
  {
    client.Reply(ftp::CantOpenDataConnection,
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
  
  DirectoryList dirList(client, path, ListOptions(options, ""),
                        true, cfg::Get().Lslong().MaxRecursion());

  try
  {
    dirList.Execute();    
  }
  catch (const util::net::NetworkError& e)
  {
    client.Reply(ftp::DataCloseAborted,
                "Error whiling writing to data connection: " + e.Message());
  }
  
  client.DataClose();
  client.Reply(ftp::DataClosedOkay, "End of directory listing."); 
}

void NOOPCommand::Execute()
{
  client.Reply(ftp::CommandOkay, "NOOP command successful."); 
}

void OPTSCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "OPTS Command not implemented."); 
}

void PASSCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;    
  }
  
  if (!client.VerifyPassword(argStr))
  {
    if (client.PasswordAttemptsExceeded())
    {
      client.Reply(ftp::NotLoggedIn,
                  "Password attempts exceeded, disconnecting.");
      client.SetFinished();
    }
    else
    {
      client.Reply(ftp::NotLoggedIn, "Login incorrect.");
      client.SetLoggedOut();
    }
    return;
  }
  
  client.Reply(ftp::UserLoggedIn, "User " + client.User().Name() + " logged in.");
  client.SetLoggedIn();
}

void PASVCommand::Execute()
{
  if (!argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::net::Endpoint ep;
  try
  {
    client.DataInitPassive(ep, ftp::PassiveType::PASV);
  }
  catch (const util::net::NetworkError& e)
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to listen for data connection: " + e.Message());
    return;
  }

  std::string portString;
  util::Error e = util::net::ftp::EndpointToPORT(ep, portString);
  if (!e)
  {
    client.Reply(ftp::SyntaxError, "PASV failed: " + e.Message());
    return;
  }
  
  client.Reply(ftp::PassiveMode, "Entering passive mode (" + portString + ")");
}

void PBSZCommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] != "0")
    client.Reply(ftp::ParameterNotImplemented, "Only protection buffer size 0 supported.");
  else
    client.Reply(ftp::CommandOkay, "Protection buffer size set to 0.");
}

void PORTCommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  util::net::Endpoint ep;
  util::Error e = util::net::ftp::EndpointFromPORT(args[1], ep);
  if (!e)
  {
    client.Reply(ftp::SyntaxError, "Invalid port string.");
    return;
  }
  
  try
  {
    client.DataInitActive(ep);
  }
  catch (const util::net::NetworkError& e)
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return;
  }
  
  client.Reply(ftp::CommandOkay, "PORT command successful.");
}

void PROTCommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] == "P")
  {
    client.SetDataProtected(true);
    client.Reply(ftp::CommandOkay, "Protection type set to 'private'.");
  }
  else if (args[1] == "C")
  {
    client.SetDataProtected(false);
    client.Reply(ftp::CommandOkay, "Protection type set to 'clear'.");
  }
  else if (args[1] == "S")
    client.Reply(ftp::ParameterNotImplemented,
                 "Protection type 'secure' not implemented.");
  else if (args[1] == "E")
    client.Reply(ftp::ParameterNotImplemented,
                 "Protection type 'confidential' not implemented.");
  else
    client.Reply(ftp::SyntaxError, "Unrecognised protection type.");
}

void PWDCommand::Execute()
{
  client.Reply(ftp::PathCreated, "\"" + client.WorkDir().ToString() +
               "\" is your working directory.");
}

void QUITCommand::Execute()
{
  client.Reply(ftp::ClosingControl, "Bye bye"); 
  client.SetFinished();
}

void REINCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "REIN Command not implemented."); 
}

void RESTCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "REST Command not implemented."); 
}

void RETRCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
    
  fs::InStreamPtr fin;
  try
  {
    fin = fs::OpenFile(client, argStr);
  }
  catch (const util::SystemError& e)
  {
    client.Reply(ftp::ActionNotOkay,
                 "Unable to open file: " + e.Message());
    return;
  }

  client.Reply(ftp::TransferStatusOkay,
               "Opening data connection for download of " + 
               fs::Path(argStr).Basename().ToString() + ".");

  try
  {
    client.DataOpen(ftp::TransferType::Download);
  }
  catch (const util::net::NetworkError&e )
  {
    client.Reply(ftp::CantOpenDataConnection,
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
      client.data.Write(buffer, len);
    }
  }
  catch (const std::ios_base::failure&)
  {
    fin->close();
    client.DataClose();
    client.Reply(ftp::DataCloseAborted,
                 "Error while reading from disk.");
    return;
  }
  catch (const util::net::NetworkError& e)
  {
    client.DataClose();
    client.Reply(ftp::DataCloseAborted,
                 "Error while writing to data connection: " +
                 e.Message());
    return;
  }
  
  client.DataClose();
  client.Reply(ftp::DataClosedOkay, "Transfer finished."); 
}

void RMDCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::RemoveDirectory(client,  argStr);
  if (!e) client.Reply(ftp::ActionNotOkay, "RMD failed: " + e.Message());
  else client.Reply(ftp::FileActionOkay, "RMD command successful."); 
}

void RNFRCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  fs::Path absolute = (client.WorkDir() / argStr).Expand();
  
  util::Error e(PP::FileAllowed<PP::Rename>(client.User(), absolute));
  if (!e)
  {
    client.Reply(ftp::ActionNotOkay, "RNFR failed: " + e.Message());
    return;
  }

  try
  {
    fs::Status status(absolute);
  }
  catch (const util::SystemError& e)
  {
    client.Reply(ftp::ActionNotOkay, "RNFR failed: " + e.Message());
    return;
  }
  
  client.PartReply(ftp::PendingMoreInfo, std::string(absolute));
  client.SetRenameFrom(absolute);
  client.Reply("File exists, ready for destination name."); 
}

void RNTOCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::RenameFile(client, client.RenameFrom(), argStr);
  if (!e) client.Reply(ftp::ActionNotOkay, "RNTO failed: " + e.Message());
  else client.Reply(ftp::FileActionOkay, "RNTO command successful.");
}

void SITECommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  boost::to_upper(args[1]);
  if (args[1] == "EPSV")
  {
    static const char* syntax = "Syntax: SITE EPSV normal|full";
    if (args.size() == 2)
      client.Reply(ftp::CommandOkay, "Extended passive mode is currently '" +
                   std::string(client.EPSVMode() == ftp::EPSVMode::Normal ?
                   "normal" : "full") + "'.");
                    
    else if (args.size() != 3) client.Reply(ftp::SyntaxError, syntax);
    else
    {
      boost::to_upper(args[2]);
      if (args[2] == "NORMAL")
      {
        client.SetEPSVMode(ftp::EPSVMode::Normal);
        client.Reply(ftp::SyntaxError, "Extended passive mode now set to 'normal'.");
      }
      else if (args[2] == "FULL")
      {
        client.SetEPSVMode(ftp::EPSVMode::Full);
        client.Reply(ftp::SyntaxError, "Extended passive mode now set to 'full'.");
      }
      else
        client.Reply(ftp::SyntaxError, syntax);
    }
    return;
  }
  
  client.Reply(ftp::CommandUnrecognised, "SITE " + args[1] + 
               " command unrecognised."); 
}

void SIZECommand::Execute()
{
  fs::Path absolute = (client.WorkDir() / argStr).Expand();

  util::Error e(PP::FileAllowed<PP::View>(client.User(), absolute));
  if (!e)
  {
    client.Reply(ftp::ActionNotOkay, "SIZE failed 2: " + e.Message());
    return;
  }
  
  fs::Status status;
  try
  {
    status.Reset(cfg::Get().Sitepath() + absolute);
  }
  catch (const util::SystemError& e)
  {
    client.Reply(ftp::ActionNotOkay, "SIZE failed 1: " + e.Message());
    return;
  }
  
  client.Reply(ftp::FileStatus, boost::lexical_cast<std::string>(status.Size())); 
}

void SMNTCommand::Execute()
{
  client.Reply(ftp::NotImplemented, "SMNT Command not implemented."); 
}

void STATCommand::Execute()
{
  if (args.size() == 1)
  {
    client.PartReply(ftp::SystemStatus, "FTPD status:");
    client.PartReply("< Insert status info here >");
    client.Reply("End of status.");
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
    
  client.PartReply(ftp::DirectoryStatus, "Status of " + path + ":");
  DirectoryList dirList(client, path, ListOptions(options, forcedOptions),
                        false, config.Lslong().MaxRecursion());
  dirList.Execute();
  
  client.Reply("End of status.");
}

void STORCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  fs::OutStreamPtr fout;
  try
  {
    fout = fs::CreateFile(client, argStr);
  }
  catch (const util::SystemError& e)
  {
    client.Reply(ftp::ActionNotOkay,
                 "Unable to create file: " + e.Message());
    return;
  }

  client.Reply(ftp::TransferStatusOkay,
               "Opening data connection for upload of " +
               fs::Path(argStr).Basename().ToString() + ".");

  try
  {
    client.DataOpen(ftp::TransferType::Upload);
  }
  catch (const util::net::NetworkError&e )
  {
    client.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return;
  }

  try
  {
    char buffer[16384];
    while (true)
    {
      size_t len = client.data.Read(buffer, sizeof(buffer));
      fout->write(buffer, len);
    }
  }
  catch (const util::net::EndOfStream&) { }
  catch (const util::net::NetworkError& e)
  {
    client.DataClose();
    client.Reply(ftp::DataCloseAborted,
                 "Error while reading from data connection: " +
                 e.Message());
    return;
  }
  
  client.DataClose();
  client.Reply(ftp::DataClosedOkay, "Transfer finished."); 
}

void STOUCommand::Execute()
{
  static size_t filenameLength = 10;

  if (!argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  fs::Path uniquePath;
  if (!fs::UniqueFile(client, client.WorkDir(),
                      filenameLength, uniquePath))
  {
    client.Reply(ftp::ActionNotOkay,
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
  client.Reply(ftp::NotImplemented, "STRU Command not implemented."); 
}

void SYSTCommand::Execute()
{
  client.Reply(ftp::SystemType, "UNIX Type: L8"); 
}

void TYPECommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] != "I" && args[1] != "A")
  {
    client.Reply(ftp::ParameterNotImplemented,
                 "TYPE " + args[1] + " not supported.");
    return;
  }
  
  client.Reply(ftp::CommandOkay, "TYPE command successful."); 
}

void USERCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }

  // 
  try
  {
    client.user = std::shared_ptr<acl::User>(acl::UserCache::UserPtr(argStr));
  }
  catch (const util::RuntimeError& e)
  {
    client.Reply(ftp::NotLoggedIn, "User " + argStr + " access denied.");
    return;
  }

  if (!client.user || argStr != client.User().Name())
  {
    client.Reply(ftp::NotLoggedIn, "User " + argStr + " access denied.");
    return;
  }
  
  client.Reply(ftp::NeedPassword, "Password required for " + argStr + "."); 
  client.SetWaitingPassword();
}

} /* cmd namespace */
