#include <boost/algorithm/string.hpp>
#include "cmd/site/factory.hpp"
#include "cmd/site/epsv.hpp"
#include "cmd/site/idle.hpp"
#include "cmd/site/vers.hpp"
#include "cmd/site/xdupe.hpp"
#include "cmd/site/passwd.hpp"
#include "cmd/site/chpass.hpp"
#include "cmd/site/deluser.hpp"
#include "cmd/site/readd.hpp"
#include "cmd/site/purge.hpp"
#include "cmd/site/renuser.hpp"

#include <iostream>

namespace cmd { namespace site
{

Factory Factory::factory;

Factory::Factory()
{
  defs =
  {
    { "EPSV",       { 0,  1,  "",
                      new Creator<site::EPSVCommand>(),  
                      "Syntax: SITE EPSV normal|full",
                      "Change EPSV command mode between normal and full (fxp support)"  }, },
    { "IDLE",       { 0,  1,  "",
                      new Creator<site::IDLECommand>(),
                      "Syntax: SITE IDLE [<seconds>]",
                      "Changes idle timeout" }, },
    { "VERS",       { 0,  0,  "",
                      new Creator<site::VERSCommand>(),
                      "Syntax: SITE VERS",
                      "Display server version" }, },
    { "XDUPE",      { 0,  1,  "",
                      new Creator<site::XDUPECommand>(),
                      "Syntax: SITE XDUPE [<mode>]",
                      "Display / change extended dupe mode" }, },
    { "PASSWD",     { 1,  1,  "passwd",
                      new Creator<site::PASSWDCommand>(),
                      "Syntax: SITE PASSWD <password>",
                      "Change your password" }, },
    { "CHPASS",     { 2,  2,  "chpass",
                      new Creator<site::CHPASSCommand>(),
                      "Syntax: SITE CHPASS <user> <password>",
                      "Change another user's password" }, },
    { "DELUSER",    { 1,  1,  "deluser",
                      new Creator<site::DELUSERCommand>(),
                      "Syntax: SITE DELUSER <user>",
                      "Delete a user" }, },
    { "READD",      { 1,  1,  "readd",
                      new Creator<site::READDCommand>(),
                      "Syntax: SITE READD <user>",
                      "Readd a deleted user" }, },
    { "PURGE",      { 1,  1,  "purge",
                      new Creator<site::PURGECommand>(),
                      "Syntax: SITE PURGE <user>",
                      "Purge a deleted user" }, },
    { "RENUSER",    { 2,  2,  "renuser",
                      new Creator<site::RENUSERCommand>(),
                      "Syntax: SITE RENUSER <old user> <new user>",
                      "Rename a user" }, }
  };
}

CommandDefOptRef Factory::Lookup(const std::string& command)
{
  CommandDefsMap::const_iterator it = factory.defs.find(command);
  if (it != factory.defs.end()) return CommandDefOptRef(it->second);
  return CommandDefOptRef();
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
