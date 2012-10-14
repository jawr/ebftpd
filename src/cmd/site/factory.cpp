#include <boost/algorithm/string.hpp>
#include "cmd/site/factory.hpp"
#include "cmd/site/epsv.hpp"
#include "cmd/site/idle.hpp"
#include "cmd/site/vers.hpp"
#include "cmd/site/xdupe.hpp"
#include "cmd/site/passwd.hpp"

namespace cmd { namespace site
{

Factory Factory::factory;

Factory::Factory()
{
  Register("EPSV", new Creator<site::EPSVCommand>());
  Register("IDLE", new Creator<site::IDLECommand>());
  Register("VERS", new Creator<site::VERSCommand>());
  Register("XDUPE", new Creator<site::XDUPECommand>());
  Register("PASSWD", new Creator<site::PASSWDCommand>());
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
                              const Args& args)
{
  std::string cmd = args[0];
  boost::to_upper(cmd);
  CreatorsMap::const_iterator it = factory.creators.find(cmd);
  if (it == factory.creators.end()) return nullptr;
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
