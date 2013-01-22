#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include "cmd/site/customcommand.hpp"
#include "cmd/splitargs.hpp"
#include "cmd/site/factory.hpp"
#include "logs/logs.hpp"
#include "acl/allowsitecmd.hpp"
#include "util/processreader.hpp"
#include "exec/reader.hpp"
#include "cmd/error.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace site
{

void CustomEXECCommand::Execute()
{
  util::ProcessReader::ArgvType argv(args.begin() + 1, args.end());
  argv.insert(argv.begin(), custSiteCmd.Target());
  try
  {
    exec::Reader reader(client, argv);
    try
    {
      std::string line;
      int maxLineCount = cfg::Get().MaxSitecmdLines();
      int lineCount = 0;
      while ((maxLineCount < 0 || ++lineCount <= maxLineCount) && reader.Getline(line))
        control.PartReply(ftp::CommandOkay, line);
    }
    catch (const util::SystemError& e)
    {
      control.Reply(ftp::CommandOkay, "Error while reading from pipe: " + e.Message());
      logs::error << "Error while reading from child process pipe: "
                  << boost::join(argv, " ") 
                  << ": " << e.Message() << logs::endl;
      return;
    }
    reader.Close();
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, "Unable to execute command: " + e.Message());
    logs::error << "Failed to execute custom site command: " 
                << boost::join(argv, " ") 
                << ": " << e.Message() << logs::endl;
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
  std::vector<std::string> tArgs;
  cmd::SplitArgs(custSiteCmd.Target(), tArgs);
  cmd::SplitArgs(argStr, args);
  args.insert(args.begin(), tArgs.begin(), tArgs.end());
  
  cmd::site::CommandDefOpt def(cmd::site::Factory::Lookup(args[0], true));
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
