#include <cassert>
#include <memory>
#include <boost/algorithm/string/split.hpp>
#include "cmd/site/passwd.hpp"
#include "cmd/site/factory.hpp"

namespace cmd { namespace site
{

void PASSWDCommand::Execute()
{
  static const char* syntax = "Syntax: SITE PASSWD <password>";
  
  if (args.size() != 2)
    control.Reply(ftp::SyntaxError, syntax);
  else
  {
    std::string cpArgStr("CHPASS ");
    cpArgStr += client.User().Name();
    cpArgStr += " ";
    cpArgStr += args[1];
    
    std::vector<std::string> cpArgs;
    boost::split(cpArgs, cpArgStr, boost::is_any_of(" "));

    std::unique_ptr<cmd::Command>
      command(cmd::site::Factory::Create(client, cpArgStr, cpArgs));
    assert(command.get());
    command->Execute();
  }
}

} /* site namespace */
} /* cmd namespace */
