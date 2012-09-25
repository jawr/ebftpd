#include "cmd/factory.hpp"
#include "cmd/command.hpp"
#include <boost/algorithm/string.hpp>

namespace cmd
{

Factory Factory::factory;

Factory::Factory()
{
  Register("ABOR", new Creator<ABORCommand>(ftp::AnyState));
  Register("ACCT", new Creator<ACCTCommand>(ftp::LoggedIn));
  Register("ADAT", new Creator<ADATCommand>(ftp::LoggedIn));
  Register("ALLO", new Creator<ALLOCommand>(ftp::LoggedIn));
  Register("APPE", new Creator<APPECommand>(ftp::LoggedIn));
  Register("AUTH", new Creator<AUTHCommand>(ftp::LoggedOut));
  Register("CCC", new Creator<CCCCommand>(ftp::LoggedIn));
  Register("CDUP", new Creator<CDUPCommand>(ftp::LoggedIn));
  Register("CONF", new Creator<CONFCommand>(ftp::LoggedIn));
  Register("CWD", new Creator<CWDCommand>(ftp::LoggedIn));
  Register("DELE", new Creator<DELECommand>(ftp::LoggedIn));
  Register("ENC", new Creator<ENCCommand>(ftp::LoggedIn));
  Register("EPRT", new Creator<EPRTCommand>(ftp::LoggedIn));
  Register("EPSV", new Creator<EPSVCommand>(ftp::LoggedIn));
  Register("FEAT", new Creator<FEATCommand>(ftp::AnyState));
  Register("HELP", new Creator<HELPCommand>(ftp::AnyState));
  Register("LANG", new Creator<LANGCommand>(ftp::LoggedIn));
  Register("LIST", new Creator<LISTCommand>(ftp::LoggedIn));
  Register("LPRT", new Creator<LPRTCommand>(ftp::LoggedIn));
  Register("LPSV", new Creator<LPSVCommand>(ftp::LoggedIn));
  Register("MDTM", new Creator<MDTMCommand>(ftp::LoggedIn));
  Register("MIC", new Creator<MICCommand>(ftp::LoggedIn));
  Register("MKD", new Creator<MKDCommand>(ftp::LoggedIn));
  Register("MLSD", new Creator<MLSDCommand>(ftp::LoggedIn));
  Register("MLST", new Creator<MLSTCommand>(ftp::LoggedIn));
  Register("MODE", new Creator<MODECommand>(ftp::LoggedIn));
  Register("NLST", new Creator<NLSTCommand>(ftp::LoggedIn));
  Register("NOOP", new Creator<NOOPCommand>(ftp::AnyState));
  Register("OPTS", new Creator<OPTSCommand>(ftp::LoggedIn));
  Register("PASS", new Creator<PASSCommand>(ftp::WaitingPassword));
  Register("PASV", new Creator<PASVCommand>(ftp::LoggedIn));
  Register("PBSZ", new Creator<PBSZCommand>(ftp::LoggedIn));
  Register("PORT", new Creator<PORTCommand>(ftp::LoggedIn));
  Register("PROT", new Creator<PROTCommand>(ftp::LoggedIn));
  Register("PWD", new Creator<PWDCommand>(ftp::LoggedIn));
  Register("QUIT", new Creator<QUITCommand>(ftp::AnyState));
  Register("REIN", new Creator<REINCommand>(ftp::LoggedIn));
  Register("REST", new Creator<RESTCommand>(ftp::LoggedIn));
  Register("RETR", new Creator<RETRCommand>(ftp::LoggedIn));
  Register("RMD", new Creator<RMDCommand>(ftp::LoggedIn));
  Register("RNFR", new Creator<RNFRCommand>(ftp::LoggedIn));
  Register("RNTO", new Creator<RNTOCommand>(ftp::LoggedIn));
  Register("SITE", new Creator<SITECommand>(ftp::LoggedIn));
  Register("SIZE", new Creator<SIZECommand>(ftp::LoggedIn));
  Register("SMNT", new Creator<SMNTCommand>(ftp::LoggedIn));
  Register("STAT", new Creator<STATCommand>(ftp::LoggedIn));
  Register("STOR", new Creator<STORCommand>(ftp::LoggedIn));
  Register("STOU", new Creator<STOUCommand>(ftp::LoggedIn));
  Register("STRU", new Creator<STRUCommand>(ftp::LoggedIn));
  Register("SYST", new Creator<SYSTCommand>(ftp::LoggedIn));
  Register("TYPE", new Creator<TYPECommand>(ftp::LoggedIn));
  Register("USER", new Creator<USERCommand>(ftp::LoggedOut));
  Register("XCUP", new Creator<XCUPCommand>(ftp::LoggedIn));
  Register("XMKD", new Creator<XMKDCommand>(ftp::LoggedIn));
  Register("XPWD", new Creator<XPWDCommand>(ftp::LoggedIn));
  Register("XRCP", new Creator<XRCPCommand>(ftp::LoggedIn));
  Register("XRMD", new Creator<XRMDCommand>(ftp::LoggedIn));
  Register("XRSQ", new Creator<XRSQCommand>(ftp::LoggedIn));
  Register("XSEM", new Creator<XSEMCommand>(ftp::LoggedIn));
  Register("XSEN", new Creator<XSENCommand>(ftp::LoggedIn));
}

Factory::~Factory()
{
  while (!creators.empty())
  {
    delete creators.begin()->second;
    creators.erase(creators.begin());
  }
}

void Factory::Register(const std::string& command,
                       CreatorBase<Command>* creator)
{
  creators.insert(std::make_pair(command, creator));
}  

Command* Factory::Create(ftp::Client& client, const std::string& argStr, const Args& args,
                         ftp::ClientState& reqdState)
{
  std::string cmd = args[0];
  boost::to_upper(cmd);
  CreatorsMap::const_iterator it = factory.creators.find(cmd);
  if (it == factory.creators.end()) return 0;
  reqdState = it->second->ReqdState();
  return it->second->Create(client, argStr, args);
}


} /* cmd namespace */

#ifdef CMD_FACTORY_TEST

#include <iostream>
#include <memory>
#include "ftp/client.hpp"

int main()
{
  using namespace cmd;
  
  ftp::Client client;
  Args args;
  args.push_back("USER");
  std::auto_ptr<Command> cmd(Factory::Create(client, args));
  cmd->Execute();
  args.clear();
  args.push_back("PASS");
  cmd.reset(Factory::Create(client, args));
  cmd->Execute();
}

#endif
