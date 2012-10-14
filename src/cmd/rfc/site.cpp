#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "cmd/rfc/site.hpp"
#include "cmd/site/factory.hpp"
#include "cfg/get.hpp"
#include "acl/allowsitecmd.hpp"

namespace cmd { namespace rfc
{

void SITECommand::Execute()
{
  argStr = argStr.substr(args[0].length());
  boost::trim(argStr);
  args.erase(args.begin());
  boost::to_upper(args[0]);
  std::string aclKeyword;
  cmd::site::CommandDefOptRef def = cmd::site::Factory::Lookup(args[0]);
  if (!def)
    control.Reply(ftp::CommandUnrecognised, "Command not understood");
  else if (!acl::AllowSiteCmd(client.User(), def->ACLKeyword()))
    control.Reply(ftp::ActionNotOkay,  "SITE " + args[0] + ": Permission denied");
  else if (!def->CheckArgs(args))
    control.Reply(ftp::SyntaxError, def->Syntax());
  else
  {
    std::unique_ptr<cmd::Command> command(def->Create(client, argStr, args));
    command->Execute();
  }
}

} /* rfc namespace */
} /* cmd namespace */
