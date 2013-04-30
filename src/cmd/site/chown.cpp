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
#include <vector>
#include <string>
#include "cmd/site/chown.hpp"
#include "fs/globiterator.hpp"
#include "fs/dircontainer.hpp"
#include "cmd/error.hpp"
#include "util/path/status.hpp"
#include "util/enumbitwise.hpp"
#include "util/string.hpp"
#include "acl/group.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

void CHOWNCommand::Process(fs::VirtualPath pathmask)
{
  try
  {
    for (auto& entry : fs::GlobContainer(client.User(), pathmask, recursive))
    {
      fs::VirtualPath entryPath(pathmask.Dirname() / entry);
      try
      {
        util::path::Status status(fs::MakeReal(entryPath).ToString());
        fs::SetOwner(fs::MakeReal(entryPath), owner);
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
        "CHOWN " + pathmask.ToString() + ": " + e.Message());
  }
}

void CHOWNCommand::ParseArgs()
{
  int n = 1;
  if (util::ToLowerCopy(args[1]) == "-r") 
  {
    ++n;
    recursive = true;
  }
  
  std::vector<std::string> owners;
  util::Split(owners, args[n], ":");
  if (owners.empty() || owners.size() > 2) throw cmd::SyntaxError();
  user = owners[0];
  if (owners.size() == 2) group = owners[1];
  
  std::string::size_type pos =
      util::FindNthNonConsecutiveChar(argStr, ' ', n);
  if (pos == std::string::npos) throw cmd::SyntaxError();
  
  patharg = argStr.substr(pos);
  util::Trim(patharg);
}

void CHOWNCommand::Execute()
{
  ParseArgs();

  if (recursive && !client.ConfirmCommand(argStr))
  {
    control.Reply(ftp::CommandOkay, 
        "Repeat the command to confirm you "
        "want to do recursive chown!");
    return;
  }
  
  acl::UserID uid = -1;
  if (!user.empty())
  {
    uid = acl::NameToUID(user);
    if (uid == -1)
    {
      control.Reply(ftp::ActionNotOkay, "User " + user + " doesn't exist.");
      return;
    }
  }
  
  acl::GroupID gid = -1;
  if (!group.empty())
  {
    gid = acl::NameToGID(group);
    if (gid == -1)
    {
      control.Reply(ftp::ActionNotOkay, "Group " + group + " doesn't exist.");
      return;
    }
  }
  
  owner = fs::Owner(uid, gid);

  Process(fs::PathFromUser(patharg));
  
  std::ostringstream os;
  os << "CHOWN finished (okay on: "
     << dirs << " directories, " << files 
     << " files / failures: " << failed << ").";
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
