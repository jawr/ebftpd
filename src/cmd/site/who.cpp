#include <vector>
#include <boost/thread/future.hpp>
#include <boost/optional.hpp>
#include "cmd/site/who.hpp"
#include "acl/user.hpp"
#include "acl/userprofile.hpp"
#include "acl/userprofilecache.hpp"
#include "acl/group.hpp"
#include "acl/groupcache.hpp"
#include "ftp/listener.hpp"
#include "ftp/task/types.hpp"
#include "ftp/task/task.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "logs/logs.hpp"

namespace cmd { namespace site
{

cmd::Result WHOCommand::Execute()
{
  boost::unique_future<bool> future;
  std::vector<ftp::task::WhoUser> users;

  ftp::TaskPtr task(new ftp::task::GetOnlineUsers(users, future));
  ftp::Listener::PushTask(task);

  const cfg::Config& cfg = cfg::Get();

  future.wait();


  std::ostringstream os;
  os << "Users logged on to " << cfg.SitenameShort();
  os << "\n.-------------------------------------------------------------------------.";
  os << "\n| User        Group      Unfo               Action                        |";
  os << "\n|-----------+----------+------------------+-------------------------------|";

  acl::Group groupObj;
  std::string group;
  for (auto& user: users)
  {
    boost::optional<acl::UserProfile> profile;
    try
    {
      profile.reset(acl::UserProfileCache::UserProfile(user.user.UID()));
    }
    catch (const util::RuntimeError& e)
    {
      logs::error << "Unable to retrieve profile from user profile cache for user: " 
                  << user.user.Name() << logs::endl;
    }
    
    try
    {
      groupObj = acl::GroupCache::Group(user.user.PrimaryGID());
      group = groupObj.Name();
    }
    catch (const util::RuntimeError& e)
    {
      group = "NoGroup";
    }
    
    os << "\n| " << std::left << std::setw(9) << user.user.Name().substr(0, 9) 
       << " | " << std::left << std::setw(8) << group.substr(0, 8) 
       << " | " << std::left << std::setw(16) 
       << (profile ? profile->Tagline().substr(0, 16) : "")  << " | ";
    
    if (user.command.empty())
    {
      std::ostringstream format;
      format << "IDLE for " << user.idleTime;
      os << std::left << std::setw(29) << format.str().substr(0, 29);
    }
    else 
      os << std::left << std::setw(29) << user.command.substr(0, 29);
    os << " |";

  }

  os << "\n|-----------+----------+------------+-----+-------------------------------|";

  {
    std::ostringstream format;
    format << users.size() << " of " << cfg.TotalUsers() << " users(s) currently online.";
    os << "\n| " << std::left << std::setw(71) << format.str().substr(0, 71) << " |";
  }

  os << "\n`-------------------------------------------------------------------------'";
  control.MultiReply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay; 
}

// end
}
}
