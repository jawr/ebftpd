#include "factory.hpp"

namespace cmd
{

Factory Factory::factory;

Factory::Factory()
{
  Register("USER", new Creator<UserCommand>());
  Register("PASS", new Creator<PassCommand>());
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
  Command* cmd = Factory::Create(client, args);
  cmd->Execute();
  args.clear();
  args.push_back("PASS");
  cmd = Factory::Create(client, args);
  cmd->Execute();
  
}

#endif
