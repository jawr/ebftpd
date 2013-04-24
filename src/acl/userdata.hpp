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

#ifndef __ACL_USERDATA_HPP
#define __ACL_USERDATA_HPP

#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>
#include <boost/optional.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "acl/flags.hpp"

namespace acl
{

struct UserData
{
  acl::UserID id;
  std::string name;
  std::vector<std::string> ipMasks;

  std::string password;
  std::string salt;
  std::string flags;
  
  acl::GroupID primaryGid;
  std::vector<acl::GroupID> secondaryGids;
  std::unordered_set<acl::GroupID> gadminGids;
  
  acl::UserID creator;
  boost::gregorian::date created;

  std::unordered_map<std::string, long long> weeklyAllotment;
  std::string homeDir;
  int idleTime;
  boost::optional<boost::gregorian::date> expires;
  int numLogins;

  std::string comment;
  std::string tagline;
  
  long long maxDownSpeed;
  long long maxUpSpeed;
  int maxSimDown;
  int maxSimUp;

  int loggedIn;
  boost::optional<boost::posix_time::ptime> lastLogin;
  std::unordered_map<std::string, int> ratio;
  std::unordered_map<std::string, long long> credits;
  
  UserData() :
    id(-1),  
    primaryGid(-1),
    creator(-1),
    created(boost::gregorian::day_clock::local_day()),
    weeklyAllotment(0),
    homeDir("/"),
    idleTime(-1),
    numLogins(1),
    maxDownSpeed(0),
    maxUpSpeed(0),
    maxSimDown(-1),
    maxSimUp(-1),
    loggedIn(0)
  {
    ratio.insert(std::make_pair("", 3));
  }
};

} /* acl namespace */

#endif
