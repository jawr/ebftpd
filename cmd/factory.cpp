#include "cmd/factory.hpp"
#include "cmd/command.hpp"
#include <boost/algorithm/string.hpp>

namespace cmd
{

Factory Factory::factory;

Factory::Factory()
{
  Register("ABOR", new Creator<ABORCommand>(ftp::ClientState::AnyState));
  Register("ACCT", new Creator<ACCTCommand>(ftp::ClientState::LoggedIn));
  Register("ADAT", new Creator<ADATCommand>(ftp::ClientState::LoggedIn));
  Register("ALLO", new Creator<ALLOCommand>(ftp::ClientState::LoggedIn));
  Register("APPE", new Creator<APPECommand>(ftp::ClientState::LoggedIn));
  Register("AUTH", new Creator<AUTHCommand>(ftp::ClientState::LoggedOut));
  Register("CCC", new Creator<CCCCommand>(ftp::ClientState::LoggedIn));
  Register("CDUP", new Creator<CDUPCommand>(ftp::ClientState::LoggedIn));
  Register("CONF", new Creator<CONFCommand>(ftp::ClientState::LoggedIn));
  Register("CWD", new Creator<CWDCommand>(ftp::ClientState::LoggedIn));
  Register("DELE", new Creator<DELECommand>(ftp::ClientState::LoggedIn));
  Register("ENC", new Creator<ENCCommand>(ftp::ClientState::LoggedIn));
  Register("EPRT", new Creator<EPRTCommand>(ftp::ClientState::LoggedIn));
  Register("EPSV", new Creator<EPSVCommand>(ftp::ClientState::LoggedIn));
  Register("FEAT", new Creator<FEATCommand>(ftp::ClientState::AnyState));
  Register("HELP", new Creator<HELPCommand>(ftp::ClientState::AnyState));
  Register("LANG", new Creator<LANGCommand>(ftp::ClientState::LoggedIn));
  Register("LIST", new Creator<LISTCommand>(ftp::ClientState::LoggedIn));
  Register("LPRT", new Creator<LPRTCommand>(ftp::ClientState::LoggedIn));
  Register("LPSV", new Creator<LPSVCommand>(ftp::ClientState::LoggedIn));
  Register("MDTM", new Creator<MDTMCommand>(ftp::ClientState::LoggedIn));
  Register("MIC", new Creator<MICCommand>(ftp::ClientState::LoggedIn));
  Register("MKD", new Creator<MKDCommand>(ftp::ClientState::LoggedIn));
  Register("MLSD", new Creator<MLSDCommand>(ftp::ClientState::LoggedIn));
  Register("MLST", new Creator<MLSTCommand>(ftp::ClientState::LoggedIn));
  Register("MODE", new Creator<MODECommand>(ftp::ClientState::LoggedIn));
  Register("NLST", new Creator<NLSTCommand>(ftp::ClientState::LoggedIn));
  Register("NOOP", new Creator<NOOPCommand>(ftp::ClientState::AnyState));
  Register("OPTS", new Creator<OPTSCommand>(ftp::ClientState::LoggedIn));
  Register("PASS", new Creator<PASSCommand>(ftp::ClientState::WaitingPassword));
  Register("PASV", new Creator<PASVCommand>(ftp::ClientState::LoggedIn));
  Register("PBSZ", new Creator<PBSZCommand>(ftp::ClientState::NotBeforeAuth));
  Register("PORT", new Creator<PORTCommand>(ftp::ClientState::LoggedIn));
  Register("PROT", new Creator<PROTCommand>(ftp::ClientState::NotBeforeAuth));
  Register("PWD", new Creator<PWDCommand>(ftp::ClientState::LoggedIn));
  Register("QUIT", new Creator<QUITCommand>(ftp::ClientState::AnyState));
  Register("REIN", new Creator<REINCommand>(ftp::ClientState::LoggedIn));
  Register("REST", new Creator<RESTCommand>(ftp::ClientState::LoggedIn));
  Register("RETR", new Creator<RETRCommand>(ftp::ClientState::LoggedIn));
  Register("RMD", new Creator<RMDCommand>(ftp::ClientState::LoggedIn));
  Register("RNFR", new Creator<RNFRCommand>(ftp::ClientState::LoggedIn));
  Register("RNTO", new Creator<RNTOCommand>(ftp::ClientState::LoggedIn));
  Register("SITE", new Creator<SITECommand>(ftp::ClientState::LoggedIn));
  Register("SIZE", new Creator<SIZECommand>(ftp::ClientState::LoggedIn));
  Register("SMNT", new Creator<SMNTCommand>(ftp::ClientState::LoggedIn));
  Register("STAT", new Creator<STATCommand>(ftp::ClientState::LoggedIn));
  Register("STOR", new Creator<STORCommand>(ftp::ClientState::LoggedIn));
  Register("STOU", new Creator<STOUCommand>(ftp::ClientState::LoggedIn));
  Register("STRU", new Creator<STRUCommand>(ftp::ClientState::LoggedIn));
  Register("SYST", new Creator<SYSTCommand>(ftp::ClientState::AnyState));
  Register("TYPE", new Creator<TYPECommand>(ftp::ClientState::LoggedIn));
  Register("USER", new Creator<USERCommand>(ftp::ClientState::LoggedOut));
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
                         ftp::ClientState::Enum& reqdState)
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
  
  ftp::ClientState::Client client;
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
