#ifndef __ACL_USER_HPP
#define __ACL_USER_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <boost/optional/optional_fwd.hpp>
#include "acl/types.hpp"

namespace boost
{
namespace posix_time
{
class ptime;
}
namespace gregorian
{
class date;
}
}

namespace db
{
class User;
}

namespace acl
{

enum class Flag : char;

struct UserData;

struct ACLInfo;

class User
{
private:
  std::unique_ptr<UserData> data;
  std::unique_ptr<db::User> db;

  User();
  User(UserData&& data_);

  bool HasSecondaryGID(GroupID gid) const;
  void SetPasswordNoSave(const std::string& password);
  void CleanGadminGIDs();
  
public:
  User& operator=(User&& rhs);
  User& operator=(const User& rhs);

  User(User&& other);
  User(const User& other);
  
  ~User();

  acl::UserID ID() const;
  
  const std::string& Name() const;
  bool Rename(const std::string& name);

  const std::vector<std::string>& IPMasks() const;
  bool AddIPMask(const std::string& ipMask, std::vector<std::string>* deleted = nullptr);
  void DelIPMask(const std::string& ipMask);
  std::string DelIPMask(size_t index);
  void ClearIPMasks();

  bool VerifyPassword(const std::string& password) const;
  void SetPassword(const std::string& password);
  
  const std::string& Flags() const;
  bool HasFlags(const std::string& flags) const;
  bool HasFlag(Flag flag) const;

  void SetFlags(const std::string& flags);
  void AddFlags(const std::string& flags);
  void AddFlag(Flag flag);
  void DelFlags(const std::string& flags);
  void DelFlag(Flag flag);

  acl::GroupID PrimaryGID() const;
  std::string PrimaryGroup() const;
  const std::vector<GroupID> SecondaryGIDs() const;
  bool HasGID(GroupID gid) const;

  void SetPrimaryGID(acl::GroupID gid);
  void AddGIDs(const std::vector<acl::GroupID>& gids);
  void DelGIDs(const std::vector<acl::GroupID>& gids);
  void SetGIDs(const std::vector<acl::GroupID>& gids);
  void ToggleGIDs(const std::vector<acl::GroupID>& gids);
  
  const std::unordered_set<GroupID> GadminGIDs() const;
  bool HasGadminGID(GroupID gid) const;

  void AddGadminGID(GroupID gid);
  void DelGadminGID(GroupID gid);
  bool ToggleGadminGID(GroupID gid);

  acl::UserID Creator() const;
  const boost::gregorian::date& Created() const;
    
  const std::string& HomeDir() const;
  void SetHomeDir(const std::string& homeDir);
  
  int IdleTime() const;
  void SetIdleTime(int idleTime);
  
  const boost::optional<boost::gregorian::date>& Expires() const;
  bool Expired() const;
  void SetExpires(const boost::optional<boost::gregorian::date>& expires);
  
  int NumLogins() const;
  void SetNumLogins(int numLogins);
  
  const std::string& Comment() const;
  void SetComment(const std::string& comment);
  
  const std::string& Tagline() const;
  void SetTagline(const std::string& tagline);
  
  long long MaxDownSpeed() const;
  void SetMaxDownSpeed(long long maxDownSpeed);
  
  long long MaxUpSpeed() const;
  void SetMaxUpSpeed(long long maxUpSpeed);
  
  int MaxSimDown() const;
  void SetMaxSimDown(int maxSimDown);
  
  int MaxSimUp() const;
  void SetMaxSimUp(int maxSimUp);
  
  int LoggedIn() const;
  const boost::optional<boost::posix_time::ptime>& LastLogin() const;
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

  long long SectionWeeklyAllotment(const std::string& section) const;
  void SetSectionWeeklyAllotment(const std::string& section, long long allotment);
  
  long long DefaultWeeklyAllotment() const { return SectionWeeklyAllotment(""); }
  void SetDefaultWeeklyAllotment(long long allotment) { SetSectionWeeklyAllotment("", allotment); }

  void Purge() const;
  
  ::acl::ACLInfo ACLInfo() const;
  
  static boost::optional<User> Load(acl::UserID uid);
  static boost::optional<User> Load(const std::string& name);
  static boost::optional<User> Create(const std::string& name, const std::string& password, 
                            acl::UserID creator);
  static boost::optional<User> FromTemplate(const std::string& name, 
                            const std::string& password, acl::UserID creator, 
                            const User& templateUser);
  static std::vector<acl::UserID> GetUIDs(const std::string& multiStr = "*");
  static std::vector<acl::User> GetUsers(const std::string& multiStr = "*");
  
  static size_t TotalUsers();
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
