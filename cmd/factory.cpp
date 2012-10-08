#include <boost/algorithm/string.hpp>
#include "cmd/factory.hpp"
#include "cmd/command.hpp"
#include "cmd/rfc/abor.hpp"
#include "cmd/rfc/acct.hpp"
#include "cmd/rfc/adat.hpp"
#include "cmd/rfc/allo.hpp"
#include "cmd/rfc/appe.hpp"
#include "cmd/rfc/auth.hpp"
#include "cmd/rfc/ccc.hpp"
#include "cmd/rfc/cdup.hpp"
#include "cmd/rfc/conf.hpp"
#include "cmd/rfc/cwd.hpp"
#include "cmd/rfc/dele.hpp"
#include "cmd/rfc/enc.hpp"
#include "cmd/rfc/eprt.hpp"
#include "cmd/rfc/epsv.hpp"
#include "cmd/rfc/feat.hpp"
#include "cmd/rfc/help.hpp"
#include "cmd/rfc/lang.hpp"
#include "cmd/rfc/list.hpp"
#include "cmd/rfc/lprt.hpp"
#include "cmd/rfc/lpsv.hpp"
#include "cmd/rfc/mdtm.hpp"
#include "cmd/rfc/mic.hpp"
#include "cmd/rfc/mkd.hpp"
#include "cmd/rfc/mlsd.hpp"
#include "cmd/rfc/mlst.hpp"
#include "cmd/rfc/mode.hpp"
#include "cmd/rfc/nlst.hpp"
#include "cmd/rfc/noop.hpp"
#include "cmd/rfc/opts.hpp"
#include "cmd/rfc/pass.hpp"
#include "cmd/rfc/pasv.hpp"
#include "cmd/rfc/pbsz.hpp"
#include "cmd/rfc/port.hpp"
#include "cmd/rfc/prot.hpp"
#include "cmd/rfc/pwd.hpp"
#include "cmd/rfc/quit.hpp"
#include "cmd/rfc/rein.hpp"
#include "cmd/rfc/rest.hpp"
#include "cmd/rfc/retr.hpp"
#include "cmd/rfc/rmd.hpp"
#include "cmd/rfc/rnfr.hpp"
#include "cmd/rfc/rnto.hpp"
#include "cmd/rfc/site.hpp"
#include "cmd/rfc/size.hpp"
#include "cmd/rfc/smnt.hpp"
#include "cmd/rfc/stat.hpp"
#include "cmd/rfc/stor.hpp"
#include "cmd/rfc/stou.hpp"
#include "cmd/rfc/stru.hpp"
#include "cmd/rfc/syst.hpp"
#include "cmd/rfc/type.hpp"
#include "cmd/rfc/user.hpp"

namespace cmd
{

Factory Factory::factory;

Factory::Factory()
{
  Register("ABOR", new Creator<rfc::ABORCommand>(ftp::ClientState::AnyState));
  Register("ACCT", new Creator<rfc::ACCTCommand>(ftp::ClientState::LoggedIn));
  Register("ADAT", new Creator<rfc::ADATCommand>(ftp::ClientState::LoggedIn));
  Register("ALLO", new Creator<rfc::ALLOCommand>(ftp::ClientState::LoggedIn));
  Register("APPE", new Creator<rfc::APPECommand>(ftp::ClientState::LoggedIn));
  Register("AUTH", new Creator<rfc::AUTHCommand>(ftp::ClientState::LoggedOut));
  Register("CCC", new Creator<rfc::CCCCommand>(ftp::ClientState::LoggedIn));
  Register("CDUP", new Creator<rfc::CDUPCommand>(ftp::ClientState::LoggedIn));
  Register("CONF", new Creator<rfc::CONFCommand>(ftp::ClientState::LoggedIn));
  Register("CWD", new Creator<rfc::CWDCommand>(ftp::ClientState::LoggedIn));
  Register("DELE", new Creator<rfc::DELECommand>(ftp::ClientState::LoggedIn));
  Register("ENC", new Creator<rfc::ENCCommand>(ftp::ClientState::LoggedIn));
  Register("EPRT", new Creator<rfc::EPRTCommand>(ftp::ClientState::LoggedIn));
  Register("EPSV", new Creator<rfc::EPSVCommand>(ftp::ClientState::LoggedIn));
  Register("FEAT", new Creator<rfc::FEATCommand>(ftp::ClientState::AnyState));
  Register("HELP", new Creator<rfc::HELPCommand>(ftp::ClientState::AnyState));
  Register("LANG", new Creator<rfc::LANGCommand>(ftp::ClientState::LoggedIn));
  Register("LIST", new Creator<rfc::LISTCommand>(ftp::ClientState::LoggedIn));
  Register("LPRT", new Creator<rfc::LPRTCommand>(ftp::ClientState::LoggedIn));
  Register("LPSV", new Creator<rfc::LPSVCommand>(ftp::ClientState::LoggedIn));
  Register("MDTM", new Creator<rfc::MDTMCommand>(ftp::ClientState::LoggedIn));
  Register("MIC", new Creator<rfc::MICCommand>(ftp::ClientState::LoggedIn));
  Register("MKD", new Creator<rfc::MKDCommand>(ftp::ClientState::LoggedIn));
  Register("MLSD", new Creator<rfc::MLSDCommand>(ftp::ClientState::LoggedIn));
  Register("MLST", new Creator<rfc::MLSTCommand>(ftp::ClientState::LoggedIn));
  Register("MODE", new Creator<rfc::MODECommand>(ftp::ClientState::LoggedIn));
  Register("NLST", new Creator<rfc::NLSTCommand>(ftp::ClientState::LoggedIn));
  Register("NOOP", new Creator<rfc::NOOPCommand>(ftp::ClientState::AnyState));
  Register("OPTS", new Creator<rfc::OPTSCommand>(ftp::ClientState::LoggedIn));
  Register("PASS", new Creator<rfc::PASSCommand>(ftp::ClientState::WaitingPassword));
  Register("PASV", new Creator<rfc::PASVCommand>(ftp::ClientState::LoggedIn));
  Register("PBSZ", new Creator<rfc::PBSZCommand>(ftp::ClientState::NotBeforeAuth));
  Register("PORT", new Creator<rfc::PORTCommand>(ftp::ClientState::LoggedIn));
  Register("PROT", new Creator<rfc::PROTCommand>(ftp::ClientState::NotBeforeAuth));
  Register("PWD", new Creator<rfc::PWDCommand>(ftp::ClientState::LoggedIn));
  Register("QUIT", new Creator<rfc::QUITCommand>(ftp::ClientState::AnyState));
  Register("REIN", new Creator<rfc::REINCommand>(ftp::ClientState::LoggedIn));
  Register("REST", new Creator<rfc::RESTCommand>(ftp::ClientState::LoggedIn));
  Register("RETR", new Creator<rfc::RETRCommand>(ftp::ClientState::LoggedIn));
  Register("RMD", new Creator<rfc::RMDCommand>(ftp::ClientState::LoggedIn));
  Register("RNFR", new Creator<rfc::RNFRCommand>(ftp::ClientState::LoggedIn));
  Register("RNTO", new Creator<rfc::RNTOCommand>(ftp::ClientState::LoggedIn));
  Register("SITE", new Creator<rfc::SITECommand>(ftp::ClientState::LoggedIn));
  Register("SIZE", new Creator<rfc::SIZECommand>(ftp::ClientState::LoggedIn));
  Register("SMNT", new Creator<rfc::SMNTCommand>(ftp::ClientState::LoggedIn));
  Register("STAT", new Creator<rfc::STATCommand>(ftp::ClientState::LoggedIn));
  Register("STOR", new Creator<rfc::STORCommand>(ftp::ClientState::LoggedIn));
  Register("STOU", new Creator<rfc::STOUCommand>(ftp::ClientState::LoggedIn));
  Register("STRU", new Creator<rfc::STRUCommand>(ftp::ClientState::LoggedIn));
  Register("SYST", new Creator<rfc::SYSTCommand>(ftp::ClientState::AnyState));
  Register("TYPE", new Creator<rfc::TYPECommand>(ftp::ClientState::LoggedIn));
  Register("USER", new Creator<rfc::USERCommand>(ftp::ClientState::LoggedOut));
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
  
  ftp::ClientState::Client client;
  Args args;
  args.push_back("USER");
  std::unique_ptr<Command> cmd(Factory::Create(client, args));
  cmd->Execute();
  args.clear();
  args.push_back("PASS");
  cmd.reset(Factory::Create(client, args));
  cmd->Execute();
}

#endif
