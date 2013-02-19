#ifndef __ACL_USER_HPP
#define __ACL_USER_HPP

#include <unordered_set>
#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "acl/flags.hpp"
#include "db/dbproxy.hpp"
#include "acl/types.hpp"

namespace db
{
class User;
}

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
  
  UserData();
};

typedef db::DBProxy<UserData, acl::UserID, db::User> UserProxy;

class User
{
private:
  UserData data;
  UserProxy db;

  User();
  User(UserData&& data);

  bool HasSecondaryGID(GroupID gid) const;
  void SetPasswordNoSave(const std::string& password);
  
public:
  User& operator=(User&& rhs);
  User& operator=(const User& rhs);

  User(User&& other);
  User(const User& other);
  
  ~User();

  acl::UserID ID() const { return data.id; }
  
  const std::string& Name() const { return data.name; }
  bool Rename(const std::string& name);

  const std::vector<std::string>& IPMasks() const { return data.ipMasks; }
  bool AddIPMask(const std::string& ipMask, std::vector<std::string>* deleted = nullptr);
  void DelIPMask(const std::string& ipMask);
  void DelIPMask(size_t index);
  void ClearIPMasks();

  bool VerifyPassword(const std::string& password) const;
  void SetPassword(const std::string& password);
  
  const std::string& Flags() const { return data.flags; }
  bool HasFlags(const std::string& flags) const;
  bool HasFlag(Flag flag) const;

  void SetFlags(const std::string& flags);
  void AddFlags(const std::string& flags);
  void AddFlag(Flag flag);
  void DelFlags(const std::string& flags);
  void DelFlag(Flag flag);

  acl::GroupID PrimaryGID() const { return data.primaryGid; }
  const std::vector<GroupID> SecondaryGIDs() const { return data.secondaryGids; }
  bool HasGID(GroupID gid) const;

  void SetPrimaryGID(acl::GroupID gid);
  void AddGIDs(const std::vector<acl::GroupID>& gids);
  void DelGIDs(const std::vector<acl::GroupID>& gids);
  void SetGIDs(const std::vector<acl::GroupID>& gids);
  void ToggleGIDs(const std::vector<acl::GroupID>& gids);
  
  const std::unordered_set<GroupID> GadminGIDs() const { return data.gadminGids; }
  bool HasGadminGID(GroupID gid) const;

  void AddGadminGID(GroupID gid);
  void DelGadminGID(GroupID gid);
  bool ToggleGadminGID(GroupID gid);

  acl::UserID Creator() const { return data.creator; }
  const boost::gregorian::date& Created() const { return data.created; }
  
  long long WeeklyAllotment() const { return data.weeklyAllotment; }
  void SetWeeklyAllotment(long long weeklyAllotment);
  
  const std::string& HomeDir() const { return data.homeDir; }
  void SetHomeDir(const std::string& homeDir);
  
  int IdleTime() const { return data.idleTime; }
  void SetIdleTime(int idleTime);
  
  const boost::optional<boost::gregorian::date>& Expires() const { return data.expires; }
  bool Expired() const;
  void SetExpires(const boost::optional<boost::gregorian::date>& expires);
  
  int NumLogins() const { return data.numLogins; }
  void SetNumLogins(int numLogins);
  
  const std::string& Comment() const { return data.comment; }
  void SetComment(const std::string& comment);
  
  const std::string& Tagline() const { return data.tagline; }
  void SetTagline(const std::string& tagline);
  
  long long MaxDownSpeed() const { return data.maxDownSpeed; }
  void SetMaxDownSpeed(long long maxDownSpeed);
  
  long long MaxUpSpeed() const { return data.maxUpSpeed; }
  void SetMaxUpSpeed(long long maxUpSpeed);
  
  int MaxSimDown() const { return data.maxSimDown; }
  void SetMaxSimDown(int maxSimDown);
  
  int MaxSimUp() const { return data.maxSimUp; }
  void SetMaxSimUp(int maxSimUp);
  
  int LoggedIn() const { return data.loggedIn; }
  const boost::optional<boost::posix_time::ptime>& LastLogin() const { return data.lastLogin; }
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
  void IncrDefaultCredits(long long kBytes)
  { IncrSectionCredits("", kBytes); }
  bool DecrDefaultCredits(long long kBytes)
  { return DecrSectionCredits("", kBytes); }
  void DecrDefaultCreditsForce(long long kBytes)
  { return DecrSectionCreditsForce("", kBytes); }
  
  void Purge() const;
  
  static boost::optional<User> Load(acl::UserID uid);
  static boost::optional<User> Load(const std::string& name);
  static boost::optional<User> Create(const std::string& name, const std::string& password, 
                                      acl::UserID creator);
  static User FromTemplate(const std::string& name, const std::string& password,
                           acl::UserID creator, const User& templateUser);
  static std::vector<acl::UserID> GetUIDs(const std::string& multiStr = "*");
  static std::vector<acl::User> GetUsers(const std::string& multiStr = "*");
};

std::string UIDToName(acl::UserID uid);
acl::UserID NameToUID(const std::string& name);
acl::GroupID UIDToPrimaryGID(acl::UserID uid);

inline acl::GroupID NameToPrimaryGID(const std::string& name)
{ return UIDToPrimaryGID(NameToUID(name)); }

inline bool UIDExists(acl::UserID uid)
{ return UIDToName(uid) != "unknown"; }

inline bool UserExists(const std::string& name)
{ return NameToUID(name) != -1; }

} /* acl namespace */

#endif
