#ifndef __ACL_USERPROFILE_HPP
#define __ACL_USERPROFILE_HPP

#include <unordered_set>
#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "acl/recode/user.hpp"
#include "acl/flags.hpp"

namespace db { namespace recode
{
class UserProfileDB;
}
}

namespace acl { namespace recode
{

class UserProfile;

typedef DBProxy<UserProfile, acl::UserID, db::recode::UserProfileDB> UserProfileDBProxy;

class UserProfile : public User
{
  UserProfileDBProxy db;

  std::string password;
  std::string salt;
  std::string flags;
  
  acl::GroupID primaryGid;
  std::vector<acl::GroupID> secondaryGids;
  std::unordered_set<acl::GroupID> gadminGids;
  
  acl::UserID creator;
  boost::gregorian::date created;

  long long weeklyAllotment;
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

public:
  UserProfile();
  UserProfile(const User& user);
  
  ~UserProfile();
  
  bool VerifyPassword(const std::string& password) const;
  void SetPassword(const std::string& password);
  
  const std::string& Flags() const { return flags; }
  bool CheckFlags(const std::string& flags) const;
  bool CheckFlag(Flag flag) const;

  void SetFlags(const std::string& flags);
  void AddFlags(const std::string& flags);
  void AddFlag(Flag flag);
  void DelFlags(const std::string& flags);
  void DelFlag(Flag flag);

  acl::UserID PrimaryGID() const { return primaryGid; }
  const std::vector<GroupID> SecondaryGIDs() const { return secondaryGids; }
  bool HasSecondaryGID(GroupID gid) const;
  bool HasGID(GroupID gid) const;

  void SetPrimaryGID(acl::GroupID gid);
  void AddSecondaryGID(GroupID gid);
  void DelSecondaryGID(GroupID gid);
  void ResetSecondaryGIDs();
  
  const std::unordered_set<GroupID> GadminGIDs() const { return gadminGids; }
  bool HasGadminGID(GroupID gid) const;

  void AddGadminGID(GroupID gid);
  void DelGadminGID(GroupID gid);

  acl::UserID Creator() const { return creator; }
  const boost::gregorian::date& Created() const { return created; }
  
  long long WeeklyAllotment() const { return weeklyAllotment; }
  void SetWeeklyAllotment(long long weeklyAllotment);
  
  const std::string& HomeDir() const { return homeDir; }
  void SetHomeDir(const std::string& homeDir);
  
  int IdleTime() const { return idleTime; }
  void SetIdleTime(int idleTime);
  
  const boost::optional<boost::gregorian::date>& Expires() const { return expires; }
  bool Expired() const;
  void SetExpires(const boost::optional<boost::gregorian::date>& expires);
  
  int NumLogins() const { return numLogins; }
  void SetNumLogins(int numLogins);
  
  const std::string& Comment() const { return comment; }
  void Comment(const std::string& comment);
  
  const std::string& Tagline() const { return tagline; }
  void Tagline(const std::string& tagline);
  
  long long MaxDownSpeed() const { return maxDownSpeed; }
  void SetMaxDownSpeed(long long maxDownSpeed);
  
  long long MaxUpSpeed() const { return maxUpSpeed; }
  void SetMaxUpSpeed(long long maxUpSpeed);
  
  int MaxSimDown() const { return maxSimDown; }
  void SetMaxSimDown(int maxSimDown);
  
  int MaxSimUp() const { return maxSimUp; }
  void SetMaxSimUp(int maxSimUp);
  
  int LoggedIn() const { return loggedIn; }
  const boost::optional<boost::posix_time::ptime>& LastLogin() const { return lastLogin; }
  void SetLoggedIn();
  
  int SectionRatio(const std::string& section) const;
  void SetSectionRatio(const std::string& section, int ratio);
  
  int DefaultRatio() const { return SectionRatio(""); }
  void SetDefaultRatio(int ratio) { SetSectionRatio("", ratio); }
  
  long long SectionCredits(const std::string& section) const;
  void IncrSectionCredits(const std::string& section, long long kBytes);
  bool DecrSectionCredits(const std::string& section, long long kBytes);
  void DecrSectionCreditsForce(const std::string& section, long long kBytes);
  
  long long DefaultCredits() const { return SectionCredits(""); }
  void IncrDefaultCredits(const std::string& section, long long kBytes)
  { IncrSectionCredits("", kBytes); }
  bool DecrDefaultCredits(const std::string& section, long long kBytes)
  { return DecrDefaultCredits(section, kBytes); }
  void DecrDefaultCreditsForce(const std::string& section, long long kBytes)
  { return DecrDefaultCreditsForce("", kBytes); }
  
  friend class DBProxy<UserProfile, acl::UserID, db::recode::UserProfileDB>;
};

} /* recode namespace */
} /* acl namespace */

#endif
