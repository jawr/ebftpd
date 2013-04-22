//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "util/string.hpp"
#include "cmd/site/customcommand.hpp"
#include "cmd/util.hpp"
#include "cmd/site/factory.hpp"
#include "logs/logs.hpp"
#include "acl/misc.hpp"
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
      logs::Error("Error while reading from child process pipe: %1%: %2%",
                  util::Join(argv, " "), e.Message());
      return;
    }
    reader.Close();
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, "Unable to execute command: " + e.Message());
    logs::Error("Failed to execute custom site command: %1%: %2%",
                util::Join(argv, " "), e.Message());
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
  args.insert(args.begin() + 1, tArgs.begin() + 1, tArgs.end());
  
  cmd::site::CommandDefOpt def(cmd::site::Factory::Lookup(tArgs[0], true));
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
        control.Format(ftp::SyntaxError, "Syntax: %1% [<arguments> ..]", args[0]);
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
