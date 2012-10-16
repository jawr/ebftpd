#include "cmd/site/change.hpp"
#include "acl/userprofilecache.hpp"
#include "logs/logs.hpp"

namespace cmd { namespace site
{

cmd::Result CHANGECommand::Execute()
{
  acl::User user;
  try
  {
    user = acl::UserCache::User(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return cmd::Result::Okay;
  }

  std::string setting = args[2];
  std::string value = args[3];
  
  if (args[0] != "CHANGE")
  {
    setting = args[0];
    value = args[2];
  }

  util::Error ok;

  logs::debug << args[0] << " " << args[1] << " " << args[2] << " Setting: " << setting << " Value: " << value << logs::endl;

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
  else
  {
    control.Reply(ftp::ActionNotOkay, "Error: " + setting + " field not found!");
    return cmd::Result::Okay;
  }

  if (!ok)
    control.Reply(ftp::ActionNotOkay, "Error: " + ok.Message());
  else
    control.Reply(ftp::CommandOkay, "Updated " + user.Name() + " " + setting + " to " + value + ".");

  return cmd::Result::Okay;
}

// end
}
}
