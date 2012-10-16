#ifndef __ACL_USERPROFILE_HPP
#define __ACL_USERPROFILE_HPP

#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>

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

  int8_t ratio;
  signed int weeklyAllotment;
  std::string homeDir;
  std::string startupDir;
  signed int idleTime;
  boost::gregorian::date expires;
  signed int numLogins; // used to be # #, with the second int being logins
  // from the same ip.

  std::string tagline;
  std::string comment;
  
  int maxDlSpeed;
  int maxUlSpeed;
  int maxSimDl;
  int maxSimUl;

public:
  UserProfile(acl::UserID uid); 

  util::Error SetRatio(int8_t ratio);
  util::Error SetWeeklyAllotment(signed int weeklyAllotment);
  util::Error SetHomeDir(const std::string& homeDir);
  util::Error SetStartupDir(const std::string& startupDir);
  util::Error SetIdleTime(signed int idleTime);
  util::Error SetExpires(const std::string& date);
  util::Error SetNumLogins(signed int numLogins);
  util::Error SetTagline(const std::string& tagline);
  util::Error SetComment(const std::string& comment);
  util::Error SetMaxDlSpeed(int maxDlSpeed);
  util::Error SetMaxUlSpeed(int maxUlSpeed);
  util::Error SetMaxSimDl(int maxSimDl);
  util::Error SetMaxSimUl(int maxSimUl);


  int8_t Ratio() const { return ratio; }
  signed int WeeklyAllotment() const { return weeklyAllotment; }
  const std::string& HomeDir() const { return homeDir; }
  const std::string& StartupDir() const { return startupDir; }
  signed int IdleTime() const { return idleTime; }
  const std::string Expires() const 
  { 
    return boost::gregorian::to_iso_extended_string(expires);
  }
  bool Expired(const std::string& date) const;
  signed int NumLogins() const { return numLogins; }
  const std::string& Tagline() const { return tagline; }
  const std::string& Comment() const { return comment; }
  int MaxDlSpeed() const { return maxDlSpeed; }
  int MaxUlSpeed() const { return maxUlSpeed; }
  int MaxSimDl() const { return maxSimDl; }
  int MaxSimUl() const { return maxSimUl; }

  friend struct db::bson::UserProfile;
};

// end
}
#endif
