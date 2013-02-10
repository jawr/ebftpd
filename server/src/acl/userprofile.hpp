#ifndef __ACL_USERPROFILE_HPP
#define __ACL_USERPROFILE_HPP

#include <string>
#include <unordered_map>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include "acl/types.hpp"
#include "util/error.hpp"


namespace db { namespace bson
{
struct UserProfile;
}
}

namespace acl
{

class UserProfile
{
  UserID uid;
  UserID creator;
  boost::gregorian::date created;

  long long weeklyAllotment;
  std::string homeDir;
  std::string startupDir;
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
  
public:
  UserProfile() :
    uid(-1),
    creator(0),
    weeklyAllotment(0),
    homeDir("/"),
    startupDir("/"),
    idleTime(-1),
    numLogins(1),
    maxDownSpeed(-1),
    maxUpSpeed(-1),
    maxSimDown(-1),
    maxSimUp(-1),
    loggedIn(-1)
  {
  }
  
  UserProfile(acl::UserID uid, acl::UserID creator) :
    uid(uid),
    creator(creator),
    weeklyAllotment(0),
    homeDir("/"),
    startupDir("/"),
    idleTime(-1),
    numLogins(3),
    maxDownSpeed(0),
    maxUpSpeed(0),
    maxSimDown(2),
    maxSimUp(2),
    loggedIn(0)
  {
    ratio.insert(std::make_pair("", 3));
    credits.insert(std::make_pair("", 0));
  }

  const boost::optional<boost::gregorian::date>& Expires() const { return expires; }
  bool Expired() const;
  acl::UserID Creator() const { return creator; }
  long long WeeklyAllotment() const { return weeklyAllotment; }
  const std::string& HomeDir() const { return homeDir; }
  const std::string& StartupDir() const { return startupDir; }
  int IdleTime() const { return idleTime; }
  int NumLogins() const { return numLogins; }
  const std::string& Comment() const { return comment; }
  const std::string& Tagline() const { return tagline; }
  long long MaxDownSpeed() const { return maxDownSpeed; }
  long long MaxUpSpeed() const { return maxUpSpeed; }
  int MaxSimDown() const { return maxSimDown; }
  int MaxSimUp() const { return maxSimUp; }
  acl::UserID UID() const { return uid; }
  const boost::gregorian::date& Created() const { return created; }
  const boost::optional<boost::posix_time::ptime>& LastLogin() const { return lastLogin; }
  int LoggedIn() const { return loggedIn; }
  
  int Ratio(const std::string& section) const
  {
    try
    {
      return ratio.at(section);
    }
    catch (const std::out_of_range&)
    {
      return -1;
    }
  }
  
  long long Credits(const std::string& section) const
  {
    try
    {
      return credits.at(section);
    }
    catch (const std::out_of_range&)
    {
      return 0;
    }
  }

  friend struct db::bson::UserProfile;
};

// end
}
#endif
