#include <boost/algorithm/string.hpp>
#include "cmd/site/factory.hpp"
#include "cmd/site/epsv.hpp"
#include "cmd/site/idle.hpp"
#include "cmd/site/vers.hpp"
#include "cmd/site/xdupe.hpp"
#include "cmd/site/passwd.hpp"
#include "cmd/site/deluser.hpp"
#include "cmd/site/readd.hpp"
#include "cmd/site/purge.hpp"
#include "cmd/site/renuser.hpp"
#include "cmd/site/adduser.hpp"
#include "cmd/site/addip.hpp"
#include "cmd/site/chpass.hpp"
#include "cmd/site/delip.hpp"
#include "cmd/site/deluser.hpp"

namespace cmd { namespace site
{

Factory Factory::factory;

Factory::Factory()
{
  Register("EPSV", new Creator<site::EPSVCommand>("epsv"));
  Register("IDLE", new Creator<site::IDLECommand>());
  Register("VERS", new Creator<site::VERSCommand>());
  Register("XDUPE", new Creator<site::XDUPECommand>());
  Register("PASSWD", new Creator<site::PASSWDCommand>("passwd"));
  Register("CHPASS", new Creator<site::CHPASSCommand>("chpass"));
  Register("DELUSER", new Creator<site::DELUSERCommand>("deluser"));
  Register("READD", new Creator<site::READDCommand>("readd"));
  Register("PURGE", new Creator<site::PURGECommand>("purge"));
  Register("RENUSER", new Creator<site::RENUSERCommand>("renuser"));
  Register("ADDUSER", new Creator<site::ADDUSERCommand>());
  Register("ADDIP", new Creator<site::ADDIPCommand>());
  Register("CHPASS", new Creator<site::CHPASSCommand>());
  Register("DELIP", new Creator<site::DELIPCommand>());
  Register("DELUSER", new Creator<site::DELUSERCommand>());

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
                       CreatorBase<cmd::Command>* creator)
{
  creators.insert(std::make_pair(command, creator));
}  

cmd::Command* Factory::Create(ftp::Client& client, const std::string& argStr,
                              const Args& args, std::string& aclKeyword)
{
  std::string cmd = args[0];
  boost::to_upper(cmd);
  CreatorsMap::const_iterator it = factory.creators.find(cmd);
  if (it == factory.creators.end()) return nullptr;
  aclKeyword = it->second->ACLKeyword();
  return it->second->Create(client, argStr, args);
}

} /* site namespace */
} /* cmd namespace */

#ifdef CMD_SITE_FACTORY_TEST

#include <iostream>
#include <memory>
#include "ftp/client.hpp"

int main()
{
  using namespace cmd;
  
  ftp::ClientState::Client client;
  Args args;
  args.push_back("USER");
  std::unique_ptr<cmd::Command> cmd(Factory::Create(client, args));
  cmd->Execute();
  args.clear();
  args.push_back("PASS");
  cmd.reset(Factory::Create(client, args));
  cmd->Execute();
}

#endif
