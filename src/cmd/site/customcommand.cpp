#include <boost/algorithm/string/join.hpp>
#include "cmd/site/customcommand.hpp"
#include "cmd/splitargs.hpp"
#include "cmd/site/factory.hpp"
#include "logs/logs.hpp"
#include "acl/allowsitecmd.hpp"
#include "util/processreader.hpp"
#include "exec/reader.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void CustomEXECCommand::Execute()
{
  try
  {
    util::ProcessReader::ArgvType argv(args.begin() + 1, args.end());
    exec::Reader reader(client, custSiteCmd.Target(), argv);
    try
    {
      std::string line;
      while (reader.Getline(line)) control.PartReply(ftp::CommandOkay, line);
    }
    catch (const util::SystemError& e)
    {
      control.Reply(ftp::CommandOkay, "Error while reading from pipe: " + e.Message());
      return;
    }
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, "Unable to execute command: " + e.Message());
    return;
  }
  
  control.Reply(ftp::CommandOkay, "Command successful.");
}

void CustomTEXTCommand::Execute()
{
  control.Reply(ftp::NotImplemented, "TEXT Not implemented");
}

// this is almost identical to rfc::SITECommand::Eexecute()
// possibly this can be merged somehow
void CustomALIASCommand::Execute()
{
  cmd::site::CommandDefOpt def(cmd::site::Factory::Lookup(custSiteCmd.Target(), true));
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
  else
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
}

} /* site namespace */
} /* cmd namespace */
