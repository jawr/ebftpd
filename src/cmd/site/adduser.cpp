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
#include "cmd/site/adduser.hpp"
#include "cmd/site/commands.hpp"
#include "util/string.hpp"
#include "util/error.hpp"
#include "acl/misc.hpp"
#include "acl/passwdstrength.hpp"
#include "cmd/error.hpp"
#include "acl/util.hpp"
#include "acl/group.hpp"
#include "logs/logs.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace site
{

void ADDUSERCommand::Addips(const std::string& user, 
    const std::vector<std::string>& ips)
{
  std::string cpArgStr("ADDUSER ");
  cpArgStr += user;
  for (const std::string& ip : ips) cpArgStr += " " + ip;

  std::vector<std::string> cpArgs;
  util::Split(cpArgs, cpArgStr, " ");

  ADDIPCommand(client, cpArgStr, cpArgs).Execute();
}

void ADDUSERCommand::Execute(const std::string& groupName, bool gadmin)
{
  this->groupName = groupName;
  this->gadmin = gadmin;
  Execute();
}

void ADDUSERCommand::Execute(const acl::User& templateUser)
{
  this->templateUser.reset(templateUser);
  Execute();
}

void ADDUSERCommand::Execute()
{
  if (groupName.empty() && !templateUser &&
      args[0] == "ADDUSER" &&
      !acl::AllowSiteCmd(client.User(), "adduser") &&
      acl::AllowSiteCmd(client.User(), "addusergadmin"))
  {
    groupName = client.User().PrimaryGroup();
    if (groupName == "unknown") throw cmd::PermissionError();
    gadmin = true;
  }
  
  boost::optional<acl::Group> group;
  if (!groupName.empty())
  {
    group = acl::Group::Load(groupName);
    if (!group)
    {
      control.Reply(ftp::ActionNotOkay, "Group " + groupName + " doesn't exist.");
      throw cmd::NoPostScriptError();
    }

    if (gadmin && group->NumSlotsUsed() >= group->Slots())
    {
      control.Reply(ftp::ActionNotOkay, "Maximum number of slots exceeded for group " + groupName + ".");
      throw cmd::NoPostScriptError();
    }
  }
  
  if (!acl::Validate(acl::ValidationType::Username, args[1]))
  {
    control.Reply(ftp::ActionNotOkay, "Username contains invalid characters");
    throw cmd::NoPostScriptError();
  }

  acl::PasswdStrength strength;
  if (!acl::SecurePass(client.User(), args[2], strength))
  {
    std::ostringstream os;
    os << "Password not strong enough. Must meed the following minimum criteria:\n"
      << strength.String() << ".";
    control.Reply(ftp::ActionNotOkay, os.str());
    throw cmd::NoPostScriptError();
  }

  bool defaultTemplate = false;
  if (!templateUser)
  {
    templateUser = acl::User::Load("default");
    if (!templateUser)
    {
      control.Reply(ftp::ActionNotOkay, "Unable to load default user template.");
      throw cmd::NoPostScriptError();
    }
    defaultTemplate = true;
  }
  
  const cfg::Config& config = cfg::Get();
  if (std::find(config.BannedUsers().begin(), config.BannedUsers().end(), args[1]) !=
      config.BannedUsers().end())
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " is banned.");
    throw cmd::NoPostScriptError();
  }
  
  if (config.TotalUsers() != -1 && static_cast<int>(acl::User::TotalUsers()) >= config.TotalUsers())
  {
    control.Format(ftp::ActionNotOkay, "Maximum users added limit of %1% has been reached.", 
                   cfg::Get().TotalUsers());
    throw cmd::NoPostScriptError();
  }
  
  auto user = acl::User::FromTemplate(args[1], args[2], client.User().ID(), *templateUser);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " already exists.");
    throw cmd::NoPostScriptError();
  }

  std::ostringstream os;
  os << "Added user " << args[1];
  
  logs::Siteop(client.User().Name(), "added user '%1%' based on template '%2%'", user->Name(), templateUser->Name());

  if (group)
  {
    user->SetPrimaryGID(group->ID());
    logs::Siteop(client.User().Name(), "set primary group for '%1%' to '%2%'", user->Name(), user->PrimaryGroup());
    os << " to group " << group->Name();
  }
  
  if (!defaultTemplate) os << " based on template " << templateUser->Name();
  
  os << ".";

  if (args.size() > 3)
  {
    control.PartReply(ftp::CommandOkay, os.str());
    Addips(args[1], std::vector<std::string>(args.begin() + 3, args.end()));
  }
  else
  {
    control.Reply(ftp::CommandOkay, os.str());
  }
}

// end
}
} 
