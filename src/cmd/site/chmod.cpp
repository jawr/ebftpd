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

#include <sstream>
#include <string>
#include "cmd/site/chmod.hpp"
#include "fs/chmod.hpp"
#include "fs/globiterator.hpp"
#include "fs/dircontainer.hpp"
#include "util/string.hpp"
#include "cmd/error.hpp"
#include "logs/logs.hpp"
#include "util/path/status.hpp"
#include "util/enumbitwise.hpp"

namespace cmd { namespace site
{

void CHMODCommand::Process(fs::VirtualPath pathmask)
{
  try
  {
    for (auto& entry : fs::GlobContainer(client.User(), pathmask, recursive))
    {
      fs::VirtualPath entryPath(pathmask.Dirname() / entry);
      try
      {
        util::path::Status status(fs::MakeReal(entryPath).ToString());
        util::Error e = fs::Chmod(client.User(), entryPath, *mode);
        if (!e)
        {
          ++failed;
          control.PartReply(ftp::CommandOkay, "CHOWN " +
              entryPath.ToString() + ": " + e.Message());
        }
        else
        if (status.IsDirectory()) ++dirs;
        else ++files;
      }
      catch (const util::SystemError& e)
      {
        ++failed;
        control.PartReply(ftp::CommandOkay, "CHOWN " +
            entryPath.ToString() + ": " + e.Message());
      }
    }
  }
  catch (const util::SystemError& e)
  {
    ++failed;
    control.PartReply(ftp::CommandOkay,
        "CHMOD " + pathmask.Dirname().ToString() + ": " + e.Message());
  }
}

void CHMODCommand::ParseArgs()
{
  int n = 1;
  util::ToLower(args[1]);
  if (args[1] == "-r")
  {
    ++n;
    recursive = true;
    util::ToLower(args[n]);
  }

  modeStr = args[n];

  std::string::size_type pos =
      util::FindNthNonConsecutiveChar(argStr, ' ', n);
  if (pos == std::string::npos) throw cmd::SyntaxError();

  patharg = argStr.substr(pos);
  util::Trim(patharg);
}

void CHMODCommand::Execute()
{
  ParseArgs();

  if (recursive && !client.ConfirmCommand(argStr))
  {
    control.Reply(ftp::CommandOkay,
        "Repeat the command to confirm you "
        "want to do recursive chmod!");
    return;
  }

  try
  {
    mode.reset(fs::Mode(modeStr));
  }
  catch (const fs::InvalidModeString& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  Process(fs::PathFromUser(patharg));

  std::ostringstream os;
  os << "CHMOD finished (okay on: "
     << dirs << " directories, " << files
     << " files / failures: " << failed << ").";
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
