#ifndef __FTP_TASK_TYPES_HPP
#define __FTP_TASK_TYPES_HPP

#include <memory>
#include <sstream>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "acl/user.hpp"
#include "ftp/transferstate.hpp"

namespace ftp { namespace task
{
class Task;

struct WhoUser
{
  acl::User user;
  ftp::TransferState tState;
  boost::posix_time::time_duration idleTime;
  std::string command;
  std::string ident;
  std::string address;
  WhoUser(const acl::User& user, const TransferState& tState, 
          const boost::posix_time::time_duration& idleTime,
          const std::string& command, const std::string& ident, 
          const std::string& address) : 
    user(user), tState(tState), idleTime(idleTime), 
    command(command), ident(ident), address(address) { }
    
  std::string Action() const;
};
// end task namespace
}

typedef std::shared_ptr<task::Task> TaskPtr;

// end
}
#endif
