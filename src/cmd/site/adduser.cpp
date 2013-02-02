#include <sstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "cmd/site/adduser.hpp"
#include "acl/usercache.hpp"
#include "util/error.hpp"
#include "cfg/get.hpp"
#include "cfg/config.hpp"
#include "acl/securepass.hpp"
#include "acl/passwdstrength.hpp"
#include "cmd/error.hpp"
#include "acl/groupcache.hpp"
#include "cmd/site/addip.hpp"
#include "acl/util.hpp"

namespace cmd { namespace site
{

void ADDUSERCommand::Addips(const std::string& user, 
    const std::vector<std::string>& ips)
{
  std::string cpArgStr("ADDIP ");
  cpArgStr += user;
  for (const std::string& ip : ips) cpArgStr += " " + ip;

  std::vector<std::string> cpArgs;
  boost::split(cpArgs, cpArgStr, boost::is_any_of(" "));

  ADDIPCommand(client, cpArgStr, cpArgs).Execute();
}

void ADDUSERCommand::Execute(const std::string& group)
{
  this->group = group;
  Execute();
}

void ADDUSERCommand::Execute()
{
  if (!acl::Validate(acl::ValidationType::Username, args[1]))
  {
    control.Reply(ftp::ActionNotOkay, "Username contains invalid characters");
    return;
  }

  acl::GroupID gid = -1;
  if (!group.empty())
  {
    gid = acl::GroupCache::NameToGID(group);
    if (gid == -1)
    {
      control.Reply(ftp::ActionNotOkay, "Group " + group + " doesn't exist.");
      return;
    }
  }

  acl::PasswdStrength strength;
  if (!acl::SecurePass(client.User(), args[2], strength))
  {
    std::ostringstream os;
    os << "Password not strong enough. Must meed the following minimum criteria:\n"
      << strength.UpperCase() << " uppercase, "
      << strength.LowerCase() << " lowercase, "
      << strength.Digits() << " digits, " 
      << strength.Others() << " others, "
      << strength.Length() << " length.";
    control.Reply(ftp::ActionNotOkay, os.str());
    throw cmd::NoPostScriptError();
  }
  
  const cfg::Config& cfg = cfg::Get();

  util::Error ok = acl::UserCache::Create(args[1], args[2], 
    cfg.DefaultFlags(), client.User().UID());

  if (!ok)
  {
    control.Reply(ftp::ActionNotOkay, ok.Message());
    throw cmd::NoPostScriptError();
  }

  std::string reply = "Added user " + args[1];
  if (gid != -1) reply += " to group " + group;
  reply += ".";
  
  if (args.size() > 3)
  {
    control.PartReply(ftp::CommandOkay, reply);
    Addips(args[1], std::vector<std::string>(args.begin() + 3, args.end()));
  }
  else
  {
    control.Reply(ftp::CommandOkay, reply);
  }
}

// end
}
} 
