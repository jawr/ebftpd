#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "cmd/rfc/site.hpp"
#include "cmd/site/factory.hpp"
#include "cfg/get.hpp"
#include "acl/allowsitecmd.hpp"
#include "cmd/splitargs.hpp"
#include "cmd/error.hpp"
#include "exec/cscript.hpp"

namespace cmd { namespace rfc
{

void SITECommand::Execute()
{
  cmd::SplitArgs(argStr, args);
  boost::to_upper(args[0]);
  argStr = argStr.substr(args[0].length());
  boost::trim(argStr);

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
      }
    }
  }
  
  return;
}

} /* rfc namespace */
} /* cmd namespace */
