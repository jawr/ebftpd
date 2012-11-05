#include <vector>
#include <boost/thread/future.hpp>
#include "cmd/site/swho.hpp"
#include "acl/user.hpp"
#include "acl/userprofile.hpp"
#include "acl/group.hpp"
#include "acl/groupcache.hpp"
#include "ftp/listener.hpp"
#include "ftp/task/types.hpp"
#include "ftp/task/task.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "db/user/userprofile.hpp"

namespace cmd { namespace site
{

cmd::Result SWHOCommand::Execute()
{
  boost::unique_future<bool> future;
  std::vector<ftp::task::WhoUser> users;

  ftp::TaskPtr task(new ftp::task::GetOnlineUsers(users, future));
  ftp::Listener::PushTask(task);

  const cfg::Config& cfg = cfg::Get();

  future.wait();


  std::ostringstream os;
  os << "Users logged on to " << cfg.SitenameShort();
  os << "\n.------------.--------------------------------.--------------------------------.";
  os << "\n| User       | Ident@Address                  | Action                         |";
  os << "\n|------------+--------------------------------+--------------------------------|";

  for (auto& user: users)
  {
    os << "\n| " << std::left << std::setw(10) << user.user.Name().substr(0, 10) << " | ";
    
    {
      std::ostringstream format;
      format << user.ident << "@" << user.address;
      
      os << std::left << std::left << std::setw(30) << format.str().substr(0, 30) << " | ";
    }
    
    os << std::left << std::setw(30) << user.Action().substr(0, 30) << " |";
  }

  os << "\n|------------+--------------------------------+--------------------------------|";

  {
    std::ostringstream format;
    format << users.size() << " of " << cfg.TotalUsers() << " users(s) currently online.";
    os << "\n| " << std::left << std::setw(76) << format.str().substr(0, 76) << " |";
  }

  os << "\n`------------------------------------------------------------------------------'";
  control.MultiReply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay; 
}

// end
}
}
