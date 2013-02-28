#include "cmd/rfc/site.hpp"
#include "cmd/site/factory.hpp"
#include "cfg/get.hpp"
#include "acl/misc.hpp"
#include "cmd/util.hpp"
#include "cmd/error.hpp"
#include "exec/cscript.hpp"
#include "util/string.hpp"
#include "logs/logs.hpp"
#include "acl/user.hpp"

namespace cmd { namespace rfc
{

void SITECommand::Execute()
{
  cmd::SplitArgs(argStr, args);
  util::ToUpper(args[0]);
  argStr = argStr.substr(args[0].length());
  util::Trim(argStr);

  std::string shortCommand = "SITE " + args[0];
  std::string fullCommand = shortCommand;
  if (!argStr.empty()) fullCommand += " " + argStr;

  cmd::site::CommandDefOpt def(cmd::site::Factory::Lookup(args[0]));
  if (!def)
  { 
    control.Reply(ftp::CommandUnrecognised, "Command not understood");
  }
  else if (!acl::AllowSiteCmd(client.User(), def->ACLKeyword()))
  {
    control.Reply(ftp::ActionNotOkay,  "SITE " + args[0] + ": Permission denied");
    logs::Security("COMMANDACL", "'%1%' attempted to run command without permission: %2%",
                   client.User().Name(), fullCommand);
  }
  else if (!def->CheckArgs(args))
  {
    control.Reply(ftp::SyntaxError, def->Syntax());
  }
  else if (exec::Cscripts(client, shortCommand, fullCommand, exec::CscriptType::PRE,
              ftp::ActionNotOkay))
  {
    cmd::CommandPtr command(def->Create(client, argStr, args));
    if (!command)
    {
      control.Reply(ftp::NotImplemented, "Command not implemented");
    }
    else
    {
      if (!util::IsASCIIOnly(argStr))
      {
        control.Reply(ftp::SyntaxError, "SITE command arguments must contain ASCII characters only");
        return;
      }
    
      try
      {
        command->Execute();
        exec::Cscripts(client, shortCommand, fullCommand, exec::CscriptType::POST, 
                ftp::ActionNotOkay);
      }
      catch (const cmd::SyntaxError&)
      {
        control.Reply(ftp::SyntaxError, def->Syntax());
      }
      catch (const cmd::PermissionError&)
      {
        control.Reply(ftp::ActionNotOkay, "SITE " + args[0] + ": Permission denied");
        logs::Security("COMMANDACL", "'%1%' attempted to run command without permission: %2%",
                       client.User().Name(), fullCommand);
      }
    }
  }
}

} /* rfc namespace */
} /* cmd namespace */
