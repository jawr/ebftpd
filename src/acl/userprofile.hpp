#ifndef __ACL_USERPROFILE_HPP
#define __ACL_USERPROFILE_HPP

#include <string>
#include <unordered_map>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/optional.hpp>
#include "acl/types.hpp"
#include "util/error.hpp"


namespace db { namespace bson
{
class UserProfile;
}
}

namespace acl
{

class UserProfile
{
  UserID uid;
  UserID creator;
  boost::gregorian::date created;

  int ratio;
  int weeklyAllotment;
  std::string homeDir;
  std::string startupDir;
  int idleTime;
  boost::optional<boost::gregorian::date> expires;
  int numLogins;

  std::string comment;
  
  int maxDlSpeed;
  int maxUlSpeed;
  int maxSimDl;
  int maxSimUl;

  int loggedIn;
  boost::optional<boost::posix_time::ptime> lastLogin;
  std::unordered_map<std::string, int> sectionRatio;
  
public:
  UserProfile() :
    uid(-1),
    creator(0),
    ratio(3),
    weeklyAllotment(0),
    homeDir("/"),
    startupDir("/"),
    idleTime(-1),
    numLogins(1),
    maxDlSpeed(-1),
    maxUlSpeed(-1),
    maxSimDl(-1),
    maxSimUl(-1),
    loggedIn(-1)
  { }
  
  UserProfile(acl::UserID uid, acl::UserID creator) :
    uid(uid),
    creator(creator),
    ratio(3),
    weeklyAllotment(0),
    homeDir("/"),
    startupDir("/"),
    idleTime(-1),
    numLogins(3),
    maxDlSpeed(0),
    maxUlSpeed(0),
    maxSimDl(2),
    maxSimUl(2),
    loggedIn(0)
  {
  }

  const boost::optional<boost::gregorian::date>& Expires() const
  { return expires; }
  bool Expired() const;
  acl::UserID Creator() const { return creator; }
  int Ratio() const { return ratio; }
  int WeeklyAllotment() const { return weeklyAllotment; }
  const std::string& HomeDir() const { return homeDir; }
  const std::string& StartupDir() const { return startupDir; }
  int IdleTime() const { return idleTime; }
  int NumLogins() const { return numLogins; }
  const std::string& Comment() const { return comment; }
  int MaxDlSpeed() const { return maxDlSpeed; }
  int MaxUlSpeed() const { return maxUlSpeed; }
  int MaxSimDl() const { return maxSimDl; }
  int MaxSimUl() const { return maxSimUl; }
  acl::UserID UID() const { return uid; }
  const boost::gregorian::date& Created() const { return created; }
  const boost::optional<boost::posix_time::ptime>& LastLogin() const { return lastLogin; }
  int LoggedIn() const { return loggedIn; }
  int SectionRatio(const std::string& section) const
  {
    try
    {
      return sectionRatio.at(section);
    }
    catch (const std::out_of_range&)
    {
      return -1;
    }
  }

  friend struct db::bson::UserProfile;
};

// end
}
#endif
