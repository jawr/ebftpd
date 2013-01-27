#include <sstream>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/algorithm/string.hpp>
#include "cmd/site/change.hpp"
#include "acl/usercache.hpp"
#include "db/user/user.hpp"
#include "db/user/userprofile.hpp"
#include "acl/allowsitecmd.hpp"
#include "util/error.hpp"
#include "cmd/error.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace site
{

void CHANGECommand::Execute()
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
  
  if (setting != "sratio" && args.size() != 4) throw cmd::SyntaxError();

  std::vector<acl::User> users;

  if (args[1] == "*")
    users = db::user::GetAll();
  else
  {
    std::vector<std::string> userToks;
    boost::trim(args[1]);
    boost::split(userToks, args[1], boost::is_any_of(" "), boost::token_compress_on);

    if (userToks.empty()) throw cmd::SyntaxError();
  
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
    throw cmd::NoPostScriptError();
  }
  
  // shouldn't change permission be checked first and override all these? i.e.
  // a user hat has access to change has access to * whereas a user who has access
  // to ratio has access only to ratio...
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
      throw cmd::NoPostScriptError();
    }
  }
  else
  if (!acl::AllowSiteCmd(client.User(), "change")) throw cmd::PermissionError();

  util::Error ok;

  for (auto& user: users)
  {
    os << "\n";

    if (setting == "ratio")
    {
      try
      {
        int ratio = boost::lexical_cast<int>(value);
        if (ratio < 0) throw boost::bad_lexical_cast();
        db::userprofile::SetRatio(user.UID(), "", ratio);
        ok = util::Error::Success();
      }
      catch (const boost::bad_lexical_cast&)
      {
        control.Reply(ftp::ActionNotOkay, "Invalid ratio");
        return;
      }
    }
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
      ok = acl::UserCache::SetTagline(user.Name(), value);
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
    else if (setting == "sratio")
    {
      const cfg::Config& config = cfg::Get();
      boost::to_upper(args[3]);
      if (config.Sections().find(args[3]) == config.Sections().end())
      {
        control.Reply(ftp::ActionNotOkay, "Section " + args[3] + " doesn't exist.");
        return;
      }
      
      try
      {
        int ratio = boost::lexical_cast<int>(args[4]);
        if (ratio < -1 || ratio > cfg::Get().MaximumRatio())
          throw boost::bad_lexical_cast();
          
        db::userprofile::SetRatio(user.UID(), args[3], ratio);
        std::ostringstream vos;
        vos << args[3] << " " << ratio;
        value = vos.str();
      }
      catch (const boost::bad_lexical_cast&)
      {
        control.Reply(ftp::ActionNotOkay, "Invalid ratio");
        return;
      }
      
      ok = util::Error::Success();
    }
    else
    {
      control.Reply(ftp::ActionNotOkay, "Invalid setting: " + setting);
      throw cmd::NoPostScriptError();
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
