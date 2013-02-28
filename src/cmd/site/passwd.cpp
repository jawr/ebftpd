#include <cassert>
#include <memory>
#include "util/string.hpp"
#include "cmd/site/passwd.hpp"
#include "cmd/site/chpass.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

void PASSWDCommand::Execute()
{
  std::string cpArgStr("PASSWD ");
  cpArgStr += client.User().Name();
  cpArgStr += " ";
  cpArgStr += args[1];

  std::vector<std::string> cpArgs;
  util::Split(cpArgs, cpArgStr, " ");
  
  CHPASSCommand(client, cpArgStr, cpArgs).Execute();
}

} /* site namespace */
} /* cmd namespace */
