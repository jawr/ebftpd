#include <sstream>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/site/change.hpp"
#include "acl/usercache.hpp"
#include "db/user/user.hpp"
#include "db/user/userprofile.hpp"
#include "acl/allowsitecmd.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void CHANGECommand::Execute()
{
  boost::ptr_vector<acl::User> users;
  std::string acl = args[1];
  if (acl[0] != '=') acl = "-" + acl;

  util::Error ok = db::user::UsersByACL(users, acl);
  if (!ok)
  {
    control.Reply(ftp::ActionNotOkay, ok.Message());
    return;
  }
  else if (users.size() == 0)
  {
    control.Reply(ftp::ActionNotOkay, "No user(s) found.");
    return;
  }

  std::ostringstream os;
  std::string setting = args[2];
  std::string value = args[3];
  
  if (args[0] != "CHANGE")
  {
    setting = args[0];
    value = args[2];
  }
  
  boost::to_lower(setting);
  
  int i = 0;
  if (users.size() > 1)
  {
    os << "Updating (" << users.size() << ") users:";
    ++i;
  }
  
  if (setting == "ratio")
  {
    if (!acl::AllowSiteCmd(client.User(), "changeratio")) throw cmd::PermissionError();
  }
  else
  if (setting == "wkly_allotment")
  {
    if (!acl::AllowSiteCmd(client.User(), "changeallot")) throw cmd::PermissionError();
  }
  else
  if (setting == "homedir")
  {
    if (!acl::AllowSiteCmd(client.User(), "changehomedir")) throw cmd::PermissionError();
  }
  else
  if (setting == "flags")
  {
    if (!acl::AllowSiteCmd(client.User(), "changeflags")) throw cmd::PermissionError();
    if (!acl::ValidFlags(value.substr(1)))
    {
      control.Reply(ftp::ActionNotOkay, "Value contains one or more invalid flags.\n"
                                        "See SITE FLAGS for a list.");
      return;
    }
  }
  else
  if (!acl::AllowSiteCmd(client.User(), "change")) throw cmd::PermissionError();

  for (auto& user: users)
  {
    if (i++ != 0) os << "\n";

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
      return;
    }

    if (!ok)
      os << ok.Message();
    else
      os << "Updated " << user.Name() << " " << setting << " to: " << value;
  }

  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
