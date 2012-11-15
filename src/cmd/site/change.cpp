#include <sstream>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/algorithm/string.hpp>
#include "cmd/site/change.hpp"
#include "acl/usercache.hpp"
#include "db/user/user.hpp"
#include "db/user/userprofile.hpp"
#include "acl/allowsitecmd.hpp"
#include "util/error.hpp"

namespace cmd { namespace site
{

cmd::Result CHANGECommand::Execute()
{
  std::ostringstream os;
  std::string setting = args[2];
  std::string value = args[3];
  
  if (args[0] != "CHANGE")
  {
    setting = args[0];
    value = args[2];
  }
  
  boost::to_lower(setting);

  std::vector<acl::User> users;

  if (args[1] == "*")
    users = db::user::GetAll();
  else
  {
    std::vector<std::string> userToks;
    boost::trim(args[1]);
    boost::split(userToks, args[1], boost::is_any_of("\t "),
      boost::token_compress_on);

    if (userToks.empty()) return cmd::Result::SyntaxError;
  
    for (auto& tok: userToks)
    {
      // could also check by flag here if need be. glftpd doesn't so for now..
      if (tok[0] == '=')
      {
        std::vector<acl::User> tempUsers = db::user::GetByACL(tok);
        users.insert(users.end(), tempUsers.begin(), tempUsers.end()); // emplace not working
      }
      else
      { 
        // user
        try
        {
          users.emplace_back(acl::UserCache::User(tok));
        }
        catch (const util::RuntimeError& e)
        {
          os << "Error: " << e.Message() << "\n";
        }
      }
    }
  }
  
  if (users.size() > 0)
    os << "Updating (" << users.size() << ") users:";
  else
  {
    control.Reply(ftp::ActionNotOkay, "Error: No users found.");
    return cmd::Result::Okay;
  }
  
  // shouldn't change permission be checked first and override all these? i.e.
  // a user hat has access to change has access to * whereas a user who has access
  // to ratio has access only to ratio...
  if (setting == "ratio")
  {
    if (!acl::AllowSiteCmd(client.User(), "changeratio")) return cmd::Result::Permission;
  }
  else
  if (setting == "wkly_allotment")
  {
    if (!acl::AllowSiteCmd(client.User(), "changeallot")) return cmd::Result::Permission;
  }
  else
  if (setting == "homedir")
  {
    if (!acl::AllowSiteCmd(client.User(), "changehomedir")) return cmd::Result::Permission;
  }
  else
  if (setting == "flags")
  {
    if (!acl::AllowSiteCmd(client.User(), "changeflags")) return cmd::Result::Permission;
    if (!acl::ValidFlags(value.substr(1)))
    {
      control.Reply(ftp::ActionNotOkay, "Value contains one or more invalid flags.\n"
                                        "See SITE FLAGS for a list.");
      return cmd::Result::Okay;
    }
  }
  else
  if (!acl::AllowSiteCmd(client.User(), "change")) return cmd::Result::Permission;

  util::Error ok;

  for (auto& user: users)
  {
    os << "\n";

    if (setting == "ratio")
      ok = db::userprofile::SetRatio(user.UID(), value);
    //else if (setting == "wkly_allotment")
      //ok = db::userprofile::SetWeeklyAllotment(user.UID(), value);
    //else if (setting == "homedir")
      //ok = db::userprofile::SetHomeDir(user.UID(), value);
    //else if (setting == "startup_dir")
      //ok = db::userprofile::SetStartupDir(user.UID(), value);
    else if (setting == "idle_time")
      ok = db::userprofile::SetIdleTime(user.UID(), value);
    else if (setting == "expires")
      ok = db::userprofile::SetExpires(user.UID(), value);
    else if (setting == "num_logins")
      ok = db::userprofile::SetNumLogins(user.UID(), value);
    else if (setting == "tagline")
      ok = db::userprofile::SetTagline(user.UID(), value);
    else if (setting == "comment")
      ok = db::userprofile::SetComment(user.UID(), value);
    else if (setting == "max_dlspeed")
      ok = db::userprofile::SetMaxDlSpeed(user.UID(), value);
    else if (setting == "max_ulspeed")
      ok = db::userprofile::SetMaxUlSpeed(user.UID(), value);
    else if (setting == "max_sim_down")
      ok = db::userprofile::SetMaxSimDl(user.UID(), value);
    else if (setting == "max_sim_up")
      ok = db::userprofile::SetMaxSimUl(user.UID(), value);
    else if (setting == "flags")
    {
      ok = util::Error::Failure("You must specify flags and a + or - operator in front.");
      if (value.length() > 1)
      {
        if (value[0] == '+')
        {
          value.assign(value.begin()+1, value.end());
          ok = acl::UserCache::AddFlags(user.Name(), value);
        }
        else if (value[0] == '-')
        {
          value.assign(value.begin()+1, value.end());
          ok = acl::UserCache::DelFlags(user.Name(), value);
        }
        else if (value[0] == '=')
        {
          value.assign(value.begin()+1, value.end());
          ok = acl::UserCache::SetFlags(user.Name(), value);
        }
      }
    }
    else
    {
      control.Reply(ftp::ActionNotOkay, "Invalid setting: " + setting);
      return cmd::Result::Okay;
    }

    if (!ok)
      os << ok.Message();
    else
      os << "Updated " << user.Name() << " " << setting << " to: " << value;
  }

  control.Reply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}

// end
}
}
