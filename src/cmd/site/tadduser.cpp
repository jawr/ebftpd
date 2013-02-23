#include <sstream>
#include "util/string.hpp"
#include "cmd/site/tadduser.hpp"
#include "cmd/site/adduser.hpp"
#include "cmd/error.hpp"
#include "acl/flags.hpp"

namespace cmd { namespace site
{

void TADDUSERCommand::Execute()
{
  auto templateUser = acl::User::Load(args[1]);
  if (!templateUser)
  {
    control.Reply(ftp::ActionNotOkay, "Template user " + args[1] + " doesn't exist.");
    throw cmd::NoPostScriptError();
  }
  
  if (!templateUser->HasFlag(acl::Flag::Template))
  {
    control.Reply(ftp::ActionNotOkay, templateUser->Name() + " is not a template user.");
    throw cmd::NoPostScriptError();
  }
  
  std::string cpArgStr("TADDUSER ");
  cpArgStr += args[2];
  for (auto it = args.begin() + 3; it != args.end(); ++it)
    cpArgStr += " " + *it;

  std::vector<std::string> cpArgs;
  util::Split(cpArgs, cpArgStr, " ");
  ADDUSERCommand(client, cpArgStr, cpArgs).Execute(*templateUser);
}

// end
}
} 

