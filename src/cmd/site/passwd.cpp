#include <cassert>
#include <memory>
#include <boost/algorithm/string/split.hpp>
#include "cmd/site/passwd.hpp"
#include "cmd/site/factory.hpp"

namespace cmd { namespace site
{

cmd::Result PASSWDCommand::Execute()
{
  std::string cpArgStr("CHPASS ");
  cpArgStr += client.User().Name();
  cpArgStr += " ";
  cpArgStr += args[1];

  std::vector<std::string> cpArgs;
  boost::split(cpArgs, cpArgStr, boost::is_any_of(" "));
  
  CommandPtr command(cmd::site::Factory::
      Lookup(cpArgs[0])->Create(client, cpArgStr, cpArgs));
  assert(command.get());
  command->Execute();
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
