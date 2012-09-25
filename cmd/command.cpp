#include <boost/lexical_cast.hpp>
#include "cmd/command.hpp"
#include "ftp/client.hpp"
#include "fs/directory.hpp"
#include "fs/file.hpp"
#include "util/error.hpp"
#include "fs/status.hpp"
#include "fs/path.hpp"

namespace cmd
{

void ABORCommand::Execute()
{
  client.Reply(226, "ABOR command successful."); 
}

void ACCTCommand::Execute()
{
  client.Reply(502, "ACCT Command not implemented."); 
}

void ADATCommand::Execute()
{
  client.Reply(502, "ADAT Command not implemented."); 
}

void ALLOCommand::Execute()
{
  client.Reply(502, "ALLO Command not implemented."); 
}

void APPECommand::Execute()
{
  client.Reply(502, "APPE Command not implemented."); 
}

void AUTHCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(500, "Wrong number of arguments.");
    return;
  }
  
  if (argStr != "TLS")
  {
    client.Reply(504, "AUTH " + argStr + " is unsupported.");
    return;
  }
  
  client.Reply(234, "AUTH TLS successful."); 
  client.NegotiateTLS();  
}

void CCCCommand::Execute()
{
  client.Reply(502, "CCC Command not implemented."); 
}

void CDUPCommand::Execute()
{
  if (!argStr.empty())
  {
    client.Reply(500, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::ChangeDirectory(client,  "..");
  if (!e) client.Reply(550, "CDUP failed: " + e.Message());
  else client.Reply(502, "CDUP command successful."); 
}

void CONFCommand::Execute()
{
  client.Reply(502, "CONF Command not implemented."); 
}

void CWDCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(500, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::ChangeDirectory(client,  argStr);
  if (!e) client.Reply(550, "CWD failed: " + e.Message());
  else client.Reply(502, "CWD command successful."); 
}

void DELECommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(500, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::DeleteFile(client,  argStr);
  if (!e) client.Reply(550, "DELE failed: " + e.Message());
  else client.Reply(502, "DELE command successful."); 
}

void ENCCommand::Execute()
{
  client.Reply(502, "ENC Command not implemented."); 
}

void EPRTCommand::Execute()
{
  client.Reply(502, "EPRT Command not implemented."); 
}

void EPSVCommand::Execute()
{
  client.Reply(502, "EPSV Command not implemented."); 
}

void FEATCommand::Execute()
{
  client.Reply(502, "FEAT Command not implemented."); 
}

void HELPCommand::Execute()
{
  static const char* reply =
    "FTP Command listing:\n"
    "------------------------------------------------------------------------\n"
    " ABOR  ACCT  ADAT  ALLO  APPE  AUTH  CCC   CDUP  CONF  CWD   DELE  ENC\n"
    " EPRT  EPSV  FEAT  HELP  LANG  LIST  LPRT  LPSV  MDTM  MIC   MKD   MLSD\n"
    " MLST  MODE  NLST  NOOP  OPTS  PASS  PASV  PBSZ  PORT  PROT  PWD   QUIT\n"
    " REIN  REST  RETR  RMD   RNFR  RNTO  SITE  SIZE  SMNT  STAT  STOR  STOU\n"
    " STRU  SYST  TYPE  USER  XCUP  XMKD  XPWD  XRCP  XRMD  XRSQ  XSEM  XSEN\n"
    "------------------------------------------------------------------------\n"
    "End of list.";
    
  client.MultiReply(214, reply);
}

void LANGCommand::Execute()
{
  client.Reply(502, "LANG Command not implemented."); 
}

void LISTCommand::Execute()
{
  client.Reply(502, "LIST Command not implemented."); 
}

void LPRTCommand::Execute()
{
  client.Reply(502, "LPRT Command not implemented."); 
}

void LPSVCommand::Execute()
{
  client.Reply(502, "LPSV Command not implemented."); 
}

void MDTMCommand::Execute()
{
  client.Reply(502, "MDTM Command not implemented."); 
}

void MICCommand::Execute()
{
  client.Reply(502, "MIC Command not implemented."); 
}

void MKDCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(500, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::CreateDirectory(client,  argStr);
  if (!e) client.Reply(550, "MKD failed: " + e.Message());
  else client.Reply(502, "MKD command successful."); 
}

void MLSDCommand::Execute()
{
  client.Reply(502, "MLSD Command not implemented."); 
}

void MLSTCommand::Execute()
{
  client.Reply(502, "MLST Command not implemented."); 
}

void MODECommand::Execute()
{
  client.Reply(502, "MODE Command not implemented."); 
}

void NLSTCommand::Execute()
{
  client.Reply(502, "NLST Command not implemented."); 
}

void NOOPCommand::Execute()
{
  client.Reply(502, "NOOP Command not implemented."); 
}

void OPTSCommand::Execute()
{
  client.Reply(502, "OPTS Command not implemented."); 
}

void PASSCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(500, "Wrong number of arguments.");
    return;    
  }
  
  if (!client.VerifyPassword(argStr))
  {
    if (client.PasswordAttemptsExceeded())
    {
      client.Reply(530, "Password attempts exceeded, disconnecting.");
      client.SetFinished();
    }
    else
    {
      client.Reply(530, "Login incorrect.");
      client.SetLoggedOut();
    }
    return;
  }
  
  client.Reply(230, "User " + client.User().Name() + " logged in.");
  client.SetLoggedIn();
}

void PASVCommand::Execute()
{
  client.Reply(502, "PASV Command not implemented."); 
}

void PBSZCommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(500, "Wrong number of arguments.");
    return;
  }
  
  // implement this properly later?
  client.Reply(200, "PBSZ command successful."); 
}

void PORTCommand::Execute()
{
  client.Reply(502, "PORT Command not implemented."); 
}

void PROTCommand::Execute()
{
  client.Reply(502, "PROT Command not implemented."); 
}

void PWDCommand::Execute()
{
  client.Reply(257, "\"" + client.WorkDir() + "\" is your working directory.");
}

void QUITCommand::Execute()
{
  client.Reply(220, "Bye bye"); 
  client.SetFinished();
}

void REINCommand::Execute()
{
  client.Reply(502, "REIN Command not implemented."); 
}

void RESTCommand::Execute()
{
  client.Reply(502, "REST Command not implemented."); 
}

void RETRCommand::Execute()
{
  client.Reply(502, "RETR Command not implemented."); 
}

void RMDCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(500, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::RemoveDirectory(client,  argStr);
  if (!e) client.Reply(550, "RMD failed: " + e.Message());
  else client.Reply(502, "RMD command successful."); 
}

void RNFRCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(500, "Wrong number of arguments.");
    return;
  }

  fs::Path absolute = (client.WorkDir() / argStr).Expand();
  
  try
  {
    fs::Status status(absolute);
  }
  catch (const util::SystemError& e)
  {
    client.Reply(550, "RNFR failed: " + e.Message());
    return;
  }
  client.PartReply(350, std::string(absolute));
  client.SetRenameFrom(absolute);
  client.Reply(350, "File exists, ready for destination name."); 
}

void RNTOCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(500, "Wrong number of arguments.");
    return;
  }
  
  util::Error e = fs::RenameFile(client, client.RenameFrom(), argStr);
  if (!e) client.Reply(550, "RNTO failed: " + e.Message());
  else client.Reply(250, "RNTO command successful.");
}

void SITECommand::Execute()
{
  client.Reply(502, "SITE Command not implemented."); 
}

void SIZECommand::Execute()
{
  fs::Path absolute = (client.WorkDir() / argStr).Expand();
  
  // check ACLs
  
  fs::Status status;
  try
  {
    status.Reset(absolute);
  }
  catch (const util::SystemError& e)
  {
    client.Reply(550, "SIZE failed: " + e.Message());
    return;
  }
  
  client.Reply(213, boost::lexical_cast<std::string>(status.Size())); 
}

void SMNTCommand::Execute()
{
  client.Reply(502, "SMNT Command not implemented."); 
}

void STATCommand::Execute()
{
  client.Reply(502, "STAT Command not implemented."); 
}

void STORCommand::Execute()
{
  client.Reply(502, "STOR Command not implemented."); 
}

void STOUCommand::Execute()
{
  client.Reply(502, "STOU Command not implemented."); 
}

void STRUCommand::Execute()
{
  client.Reply(502, "STRU Command not implemented."); 
}

void SYSTCommand::Execute()
{
  client.Reply(502, "SYST Command not implemented."); 
}

void TYPECommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(500, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] != "I" && args[1] != "A")
  {
    client.Reply(501, "TYPE " + args[1] + " not supported.");
    return;
  }
  
  client.Reply(200, "TYPE command successful."); 
}

void USERCommand::Execute()
{
  if (argStr.empty())
  {
    client.Reply(500, "Wrong number of arguments.");
    return;
  }
  
  if (argStr != client.User().Name())
  {
    client.Reply(530, "User " + argStr + " access denied.");
    return;
  }
  
  client.Reply(331, "Password required for " + argStr + "."); 
  client.SetWaitingPassword();
}

void XCUPCommand::Execute()
{
  client.Reply(502, "XCUP Command not implemented."); 
}

void XMKDCommand::Execute()
{
  client.Reply(502, "XMKD Command not implemented."); 
}

void XPWDCommand::Execute()
{
  client.Reply(502, "XPWD Command not implemented."); 
}

void XRCPCommand::Execute()
{
  client.Reply(502, "XRCP Command not implemented."); 
}

void XRMDCommand::Execute()
{
  client.Reply(502, "XRMD Command not implemented."); 
}

void XRSQCommand::Execute()
{
  client.Reply(502, "XRSQ Command not implemented."); 
}

void XSEMCommand::Execute()
{
  client.Reply(502, "XSEM Command not implemented."); 
}

void XSENCommand::Execute()
{
  client.Reply(502, "XSEN Command not implemented."); 
}

} /* cmd namespace */
