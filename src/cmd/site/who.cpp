#include <vector>
#include <boost/thread/future.hpp>
#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "cmd/site/who.hpp"
#include "acl/user.hpp"
#include "acl/userprofile.hpp"
#include "db/user/userprofile.hpp"
#include "acl/group.hpp"
#include "acl/groupcache.hpp"
#include "ftp/listener.hpp"
#include "ftp/task/types.hpp"
#include "ftp/task/task.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "logs/logs.hpp"
#include "stats/util.hpp"

namespace cmd { namespace site
{

void WHOCommand::Execute()
{
  boost::unique_future<bool> future;
  std::vector<ftp::task::WhoUser> users;

  ftp::TaskPtr task(new ftp::task::GetOnlineUsers(users, future));
  ftp::Listener::PushTask(task);

  const cfg::Config& cfg = cfg::Get();

  future.wait();

  std::ostringstream os;
  os << "Users logged on to " << cfg.SitenameShort();
  os << "\n.-----------.----------.----------------------.--------------------------------.";
  os << "\n| User      | Group    | Tagline              | Activity                       |";
  os << "\n|-----------+----------+----------------------+--------------------------------|";

  for (auto& user: users)
  {
    std::string group = acl::GroupCache::GIDToName(user.user.PrimaryGID());
    
    os << "\n| " << std::left << std::setw(9) << user.user.Name().substr(0, 9) 
       << " | " << std::left << std::setw(8) << group.substr(0, 8) 
       << " | " << std::left << std::setw(20) 
       << user.user.Tagline().substr(0, 20) << " | ";
    
    os << std::left << std::setw(30) << user.Action().substr(0, 30) << " |";

  }

  os << "\n|-----------+----------+----------------------+--------------------------------|";

  {
    std::ostringstream format;
    format << users.size() << " of " << cfg.TotalUsers() << " users(s) currently online.";
    os << "\n| " << std::left << std::setw(76) << format.str().substr(0, 76) << " |";
  }

  os << "\n`------------------------------------------------------------------------------'";
  control.Reply(ftp::CommandOkay, os.str());
  return; 
}

// end
}
}
