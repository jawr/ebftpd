#include <sstream>
#include <boost/ptr_container/ptr_vector.hpp>
#include "cmd/site/change.hpp"
#include "acl/userprofilecache.hpp"
#include "acl/usercache.hpp"
#include "db/acl/acl.hpp"

namespace cmd { namespace site
{

cmd::Result CHANGECommand::Execute()
{
  boost::ptr_vector<acl::User> users;
  std::string acl = args[1];
  if (acl[0] != '=') acl = "-" + acl;

  util::Error ok = db::GetUsersByACL(users, acl);

  if (!ok)
  {
    control.Reply(ftp::ActionNotOkay, "Error: " + ok.Message());
    return cmd::Result::Okay;
  }
  else if (users.size() == 0)
  {
    control.Reply(ftp::ActionNotOkay, "No user(s) found.");
    return cmd::Result::Okay;
  }

  std::ostringstream os;
  std::string setting = args[2];
  std::string value = args[3];
  
  if (args[0] != "CHANGE")
  {
    setting = args[0];
    value = args[2];
  }

  int i = 0;
  if (users.size() > 1)
  {
    os << "Updating (" << users.size() << ") users:";
    ++i;
  }

  for (auto& user: users)
  {
    if (i++ != 0) os << "\n";

    if (setting == "ratio" || setting == "changeratio")
      ok = acl::UserProfileCache::SetRatio(user.UID(), value);
    else if (setting == "wkly_allotment" || setting == "changeallot")
      ok = acl::UserProfileCache::SetWeeklyAllotment(user.UID(), value);
    else if (setting == "homedir" || setting == "changehomedir")
      ok = acl::UserProfileCache::SetHomeDir(user.UID(), value);
    else if (setting == "startup_dir")
      ok = acl::UserProfileCache::SetStartupDir(user.UID(), value);
    else if (setting == "idle_time")
      ok = acl::UserProfileCache::SetIdleTime(user.UID(), value);
    else if (setting == "expires")
      ok = acl::UserProfileCache::SetExpires(user.UID(), value);
    else if (setting == "num_logins")
      ok = acl::UserProfileCache::SetNumLogins(user.UID(), value);
    else if (setting == "tagline")
      ok = acl::UserProfileCache::SetTagline(user.UID(), value);
    else if (setting == "comment")
      ok = acl::UserProfileCache::SetComment(user.UID(), value);
    else if (setting == "max_dlspeed")
      ok = acl::UserProfileCache::SetMaxDlSpeed(user.UID(), value);
    else if (setting == "max_ulspeed")
      ok = acl::UserProfileCache::SetMaxUlSpeed(user.UID(), value);
    else if (setting == "max_sim_down")
      ok = acl::UserProfileCache::SetMaxSimDl(user.UID(), value);
    else if (setting == "max_sim_up")
      ok = acl::UserProfileCache::SetMaxSimUl(user.UID(), value);
    else if (setting == "flags")
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
      else
      {
        // handle setting flags if = or none?
        ok = util::Error::Failure("Not implemented. Should this set or throw sub syntax error?");
      }
    }
    else
    {
      control.Reply(ftp::ActionNotOkay, "Error: " + setting + " field not found!");
      return cmd::Result::Okay;
    }

    if (!ok)
      os << "Error: " << ok.Message();
    else
      os << "Updated " << user.Name() << " " << setting << " to " << value << ".";
  }

  control.MultiReply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}

// end
}
}
