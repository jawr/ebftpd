#include <sstream>
#include <boost/ptr_container/ptr_vector.hpp>
#include "cmd/site/change.hpp"
#include "acl/usercache.hpp"
#include "db/acl/acl.hpp"
#include "db/user/userprofile.hpp"

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
      ok = db::userprofile::SetRatio(user.UID(), value);
    else if (setting == "wkly_allotment" || setting == "changeallot")
      ok = db::userprofile::SetWeeklyAllotment(user.UID(), value);
    else if (setting == "homedir" || setting == "changehomedir")
      ok = db::userprofile::SetHomeDir(user.UID(), value);
    else if (setting == "startup_dir")
      ok = db::userprofile::SetStartupDir(user.UID(), value);
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
        static const char* changeSyntax = "you need to use + or - in front of flags.";
        ok = util::Error::Failure(changeSyntax);
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
