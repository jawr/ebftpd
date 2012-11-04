#ifndef __ACL_USERPROFILE_HPP
#define __ACL_USERPROFILE_HPP

#include <string>
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
  std::string created;

  int ratio;
  int weeklyAllotment;
  std::string homeDir;
  std::string startupDir;
  int idleTime;
  boost::optional<boost::gregorian::date> expires;
  int numLogins; // used to be # #, with the second int being logins
  // from the same ip.

  std::string tagline;
  std::string comment;
  
  int maxDlSpeed;
  int maxUlSpeed;
  int maxSimDl;
  int maxSimUl;

  int loggedIn;
  std::string lastLogin;

public:
  UserProfile() : uid(-1), creator(-1) {};
  UserProfile(acl::UserID uid, acl::UserID creator = 0);

  util::Error SetExpires(const std::string& date);

  void SetCreator(acl::UserID creator) { this->creator = creator; }


  acl::UserID Creator() const { return creator; }
  int Ratio() const { return ratio; }
  int WeeklyAllotment() const { return weeklyAllotment; }
  const std::string& HomeDir() const { return homeDir; }
  const std::string& StartupDir() const { return startupDir; }
  int IdleTime() const { return idleTime; }
  const std::string Expires() const 
  { 
    if (expires) return boost::gregorian::to_simple_string(*expires);
    else return "never";
  }
  bool Expired(const std::string& date) const;
  int NumLogins() const { return numLogins; }
  const std::string& Tagline() const { return tagline; }
  const std::string& Comment() const { return comment; }
  int MaxDlSpeed() const { return maxDlSpeed; }
  int MaxUlSpeed() const { return maxUlSpeed; }
  int MaxSimDl() const { return maxSimDl; }
  int MaxSimUl() const { return maxSimUl; }
  const acl::UserID& UID() const { return uid; }
  const std::string& Created() const { return created; }
  const std::string& LastLogin() const { return lastLogin; }
  int LoggedIn() const { return loggedIn; }

  friend struct db::bson::UserProfile;
};

// end
}
#endif
