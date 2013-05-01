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
#include "cmd/site/wipe.hpp"
#include "fs/globiterator.hpp"
#include "fs/dircontainer.hpp"
#include "fs/directory.hpp"
#include "fs/file.hpp"
#include "cmd/error.hpp"
#include "db/index/index.hpp"
#include "acl/path.hpp"
#include "util/path/status.hpp"
#include "cfg/get.hpp"
#include "util/enumbitwise.hpp"
#include "logs/logs.hpp"
#include "util/string.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{


void WIPECommand::Process(const fs::VirtualPath& pathmask)
{
  try
  {
    for (auto& entry : fs::GlobContainer(client.User(), pathmask))
    {
      fs::VirtualPath entryPath(pathmask.Dirname() / entry);
      try
      {
        util::path::Status status(fs::MakeReal(entryPath).ToString());
        if (status.IsDirectory())
        {
          Process(entryPath / "*");
          util::Error e = fs::RemoveDirectory(client.User(), entryPath);
          if (!e)
          {
            control.PartReply(ftp::CommandOkay, "WIPE " + entryPath.ToString() + ": " + e.Message());
            ++failed;
          }
          else
          {
            if (cfg::Get().IsIndexed(entryPath.ToString()))
              db::index::Delete(entryPath.ToString());
            ++dirs;
          }
        }
        else
        {
          util::Error e = fs::DeleteFile(client.User(), entryPath);
          if (!e)
          {
            control.PartReply(ftp::CommandOkay, "WIPE " + entryPath.ToString() + ": " + e.Message());
            ++failed;
          }
          else
            ++files;
        }
      }
      catch (const util::SystemError& e)
      {
        ++failed;
        control.PartReply(ftp::CommandOkay, "WIPE " + entryPath.ToString() + ": " + e.Message());
      }
    }
  }
  catch (const util::SystemError& e)
  {
    ++failed;
    control.PartReply(ftp::CommandOkay, "WIPE " + pathmask.ToString() + ": " + e.Message());
  }
  
  if (failed == 0)
  {
    logs::Event(recursive ? "WIPE-r" : "WIPE", "path", fs::MakeReal(pathmask).ToString(), 
                "user", client.User().Name(),
                "group", client.User().PrimaryGroup(), 
                "tagline", client.User().Tagline());
  }
}

void WIPECommand::ParseArgs()
{
  int n = 1;
  util::ToLower(args[1]);
  if (args[1] == "-r") 
  {
    ++n;
    recursive = true;
    patharg = argStr.substr(2);
  }
  else
    patharg = argStr;
  
  util::Trim(patharg);
}

void WIPECommand::Execute()
{
  ParseArgs();

  if (recursive && !client.ConfirmCommand(argStr))
  {
    control.Reply(ftp::CommandOkay, 
        "Repeat the command to confirm you "
        "want to do recursive wipe!");
    throw cmd::NoPostScriptError();
  }

  auto path = fs::PathFromUser(patharg);
  Process(path);
  
  std::ostringstream os;
  os << "WIPE finished (okay on: "
     << dirs << " directories, " << files 
     << " files / failures: " << failed << ").";
  control.Reply(ftp::CommandOkay, os.str());  
}

} /* site namespace */
} /* cmd namespace */
