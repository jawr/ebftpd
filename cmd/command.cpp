#include "cmd/command.hpp"
#include "ftp/client.hpp"

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
  client.Reply(502, "AUTH Command not implemented."); 
}

void CCCCommand::Execute()
{
  client.Reply(502, "CCC Command not implemented."); 
}

void CDUPCommand::Execute()
{
  client.Reply(502, "CDUP Command not implemented."); 
}

void CONFCommand::Execute()
{
  client.Reply(502, "CONF Command not implemented."); 
}

void CWDCommand::Execute()
{
  client.Reply(502, "CWD Command not implemented."); 
}

void DELECommand::Execute()
{
  client.Reply(502, "DELE Command not implemented."); 
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
  client.Reply(502, "MKD Command not implemented."); 
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
  if (args.size() != 2)
  {
    client.Reply(500, "Wrong number of arguments.");
    return;    
  }
  
  if (!client.VerifyPassword(args[1]))
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
  client.Reply(502, "PBSZ Command not implemented."); 
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
  client.Reply(502, "PWD Command not implemented."); 
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
  client.Reply(502, "RMD Command not implemented."); 
}

void RNFRCommand::Execute()
{
  client.Reply(502, "RNFR Command not implemented."); 
}

void RNTOCommand::Execute()
{
  client.Reply(502, "RNTO Command not implemented."); 
}

void SITECommand::Execute()
{
  client.Reply(502, "SITE Command not implemented."); 
}

void SIZECommand::Execute()
{
  client.Reply(502, "SIZE Command not implemented."); 
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
  client.Reply(502, "TYPE Command not implemented."); 
}

void USERCommand::Execute()
{
  if (args.size() != 2)
  {
    client.Reply(500, "Wrong number of arguments.");
    return;
  }
  
  if (args[1] != client.User().Name())
  {
    client.Reply(530, "User " + args[1] + " access denied.");
    return;
  }
  
  client.Reply(331, "Password required for " + args[1] + "."); 
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
