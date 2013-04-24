//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
  acl::UserID uid;
  ftp::TransferState tState;
  boost::posix_time::time_duration idleTime;
  std::string command;
  std::string ident;
  std::string address;
  WhoUser(const acl::UserID uid, const TransferState& tState, 
          const boost::posix_time::time_duration& idleTime,
          const std::string& command, const std::string& ident, 
          const std::string& address) : 
    uid(uid), tState(tState), idleTime(idleTime), 
    command(command), ident(ident), address(address)
  { }
    
  std::string Action() const;
};
// end task namespace
}

typedef std::shared_ptr<task::Task> TaskPtr;

// end
}
#endif
