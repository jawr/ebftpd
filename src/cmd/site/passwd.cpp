#include <cassert>
#include <memory>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "cmd/site/passwd.hpp"
#include "cmd/site/chpass.hpp"

namespace cmd { namespace site
{

void PASSWDCommand::Execute()
{
  std::string cpArgStr("CHPASS ");
  cpArgStr += client.User().Name();
  cpArgStr += " ";
  cpArgStr += args[1];

  std::vector<std::string> cpArgs;
  boost::split(cpArgs, cpArgStr, boost::is_any_of(" "));
  
  CHPASSCommand(client, cpArgStr, cpArgs).Execute();
}

} /* site namespace */
} /* cmd namespace */
