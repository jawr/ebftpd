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
  std::string startUpDir;
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
    startUpDir("/")
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
