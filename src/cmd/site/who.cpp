#include <vector>
#include <boost/thread/future.hpp>
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
    try
    {
      group.clear();
      groupObj = acl::GroupCache::Group(user.user.PrimaryGID());
      group = groupObj.Name();
    }
    catch (const util::RuntimeError& e)
    {
      group = "NoGroup";
    }
    acl::UserProfile profile = acl::UserProfileCache::UserProfile(user.user.UID());
    
    os << "\n| " << user.user.Name() << " | " << group << " | ";
    os << profile.Tagline() << " | ";
    
    if (user.command.empty())
      os << "IDLE for " << user.idleTime;
    else 
      os << user.command;
    os << " |";

  }

  os << "\n|-----------+----------+------------+-----+-------------------------------|";
  os << "\n| " << users.size() << " of " << cfg.TotalUsers();
  os << " users(s) currently online.";
  os << "\n`-------------------------------------------------------------------------'";
  control.Reply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay; 
}

// end
}
}
