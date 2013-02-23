#include <sstream>
#include "util/string.hpp"
#include "cmd/site/adduser.hpp"
#include "util/error.hpp"
#include "acl/misc.hpp"
#include "acl/passwdstrength.hpp"
#include "cmd/error.hpp"
#include "cmd/site/addip.hpp"
#include "acl/util.hpp"
#include "acl/misc.hpp"
#include "acl/group.hpp"
#include "logs/logs.hpp"

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

void ADDUSERCommand::Execute(const std::string& group)
{
  this->group = group;
  Execute();
}

void ADDUSERCommand::Execute(const acl::User& templateUser)
{
  this->templateUser.reset(templateUser);
  Execute();
}

void ADDUSERCommand::Execute()
{
  acl::GroupID gid = -1;
  if (!group.empty())
  {
    gid = acl::NameToGID(group);
    if (gid == -1)
    {
      control.Reply(ftp::ActionNotOkay, "Group " + group + " doesn't exist.");
      throw cmd::NoPostScriptError();
    }
  }
  else
  if (args[0] == "ADDUSER" &&
      !acl::AllowSiteCmd(client.User(), "adduser") &&
      acl::AllowSiteCmd(client.User(), "addusergadmin"))
  {
    gid = client.User().PrimaryGID();
    if (gid == -1) throw cmd::PermissionError();
    group = acl::GIDToName(gid);
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
  
  auto user = templateUser ?
              acl::User::FromTemplate(args[1], args[2], client.User().ID(), *templateUser) :
              acl::User::Create(args[1], args[2], client.User().ID());
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + args[1] + " already exists.");
    throw cmd::NoPostScriptError();
  }

  std::ostringstream os;
  os << "Added user " << args[1];
  
  if (templateUser)
    logs::Siteop(client.User().Name(), "TADDUSER", user->Name(), templateUser->Name());
  else
    logs::Siteop(client.User().Name(), "ADDUSER", user->Name());
  
  if (gid != -1)
  {
    user->SetPrimaryGID(gid);
    logs::Siteop(client.User().Name(), "SETPGRP", user->Name(), user->PrimaryGroup());
    os << " to group " << group;
  }
  
  if (templateUser) os << " based on template " << templateUser->Name();
  
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
