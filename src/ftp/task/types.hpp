#ifndef __FTP_TASK_TYPES_HPP
#define __FTP_TASK_TYPES_HPP

#include <memory>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "acl/user.hpp"

namespace ftp { namespace task
{
class Task;

struct WhoUser
{
  acl::User user;
  boost::posix_time::seconds idleTime;
  std::string command;
  std::string ident;
  std::string address;
  WhoUser(const acl::User& user, boost::posix_time::seconds idleTime, 
          const std::string& command, const std::string& ident, 
          const std::string& address) : 
    user(user), idleTime(idleTime), command(command), 
    ident(ident), address(address) { }
};
// end task namespace
}

typedef std::shared_ptr<task::Task> TaskPtr;

// end
}
#endif
