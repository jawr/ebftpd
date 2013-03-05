#ifndef __PLUGIN_USER_HPP
#define __PLUGIN_USER_HPP

#include <stdexcept>
#include <utility>
#include <boost/optional.hpp>
#include "acl/user.hpp"
#include "plugin/util.hpp"
#include "plugin/error.hpp"
#include "acl/util.hpp"
#include "acl/flags.hpp"
#include "plugin/locks.hpp"

namespace plugin
{

class User : public acl::User
{
  bool ValidGIDs(const std::vector<acl::GroupID>& gids)
  {
    return std::find_if(gids.begin(), gids.end(), 
              [](acl::GroupID gid) { return gid >= 0; }) == gids.end();
  }
  
public:
  explicit User(const acl::User& user) : acl::User(user) { }
  explicit User(acl::User&& user) : acl::User(std::forward<acl::User>(user)) { }
  User() { throw NotConstructable(); }

  util::Error Rename(const std::string& name)
  {
    UnlockGuard unlock; (void) unlock;
    if (!acl::Validate(acl::ValidationType::Username, name)) return util::Error::Failure("Value error");
    if (Name() == "default") return util::Error::Failure("Cannot rename default template user");
    if (!acl::User::Rename(name)) return util::Error::Failure("Already exists");
    return util::Error::Success();
  }
  
  util::Error AddIPMask(const std::string& ipMask)
  {
    UnlockGuard unlock; (void) unlock;
    if (ipMask.find('@') == std::string::npos) return util::Error::Failure("Must be in format ident@address");
    acl::User::AddIPMask(ipMask);
    return util::Error::Success();
  }
  
  void DelIPMask(const std::string& ipMask)
  {
    UnlockGuard unlock; (void) unlock;
    acl::User::DelIPMask(ipMask);
  }
  
  std::string DelIPMask(size_t index)
  {
    UnlockGuard unlock; (void) unlock;
    return acl::User::DelIPMask(index);
  }
  
  void SetPassword(const std::string& password)
  {
    UnlockGuard unlock; (void) unlock;
    acl::User::SetPassword(password);
  }
  
  void ClearIPMasks()
  {
    UnlockGuard unlock; (void) unlock;
    acl::User::ClearIPMasks();
  }
  
  util::Error SetFlags(const std::string& flags)
  {
    UnlockGuard unlock; (void) unlock;
    if (!acl::ValidFlags(flags)) return util::Error::Failure("Value error");
    acl::User::SetFlags(flags);
    return util::Error::Success();
  }
  
  util::Error AddFlags(const std::string& flags)
  {
    UnlockGuard unlock; (void) unlock;
    if (!acl::ValidFlags(flags)) return util::Error::Failure("Value error");
    acl::User::AddFlags(flags);
    return util::Error::Success();
  }
  
  void DelFlags(const std::string& flags)
  {
    UnlockGuard unlock; (void) unlock;
    acl::User::DelFlags(flags);
  }
  
  util::Error SetPrimaryGID(acl::GroupID gid)
  {
    UnlockGuard unlock; (void) unlock;
    if (gid < -1) util::Error::Failure("Value error");
    acl::User::SetPrimaryGID(gid);
    return util::Error::Success();
  }
  
  util::Error AddGIDs(const std::vector<acl::GroupID>& gids)
  {
    UnlockGuard unlock; (void) unlock;
    if (!ValidGIDs(gids)) return util::Error::Failure("Value error");
    acl::User::AddGIDs(gids);
    return util::Error::Success();
  }
  
  util::Error DelGIDs(const std::vector<acl::GroupID>& gids)
  {
    UnlockGuard unlock; (void) unlock;
    if (!ValidGIDs(gids)) return util::Error::Failure("Value error");
    acl::User::AddGIDs(gids);
    return util::Error::Success();
  }
  
  util::Error SetGIDs(const std::vector<acl::GroupID>& gids)
  {
    UnlockGuard unlock; (void) unlock;
    if (!ValidGIDs(gids)) return util::Error::Failure("Value error");
    acl::User::AddGIDs(gids);
    return util::Error::Success();
  }
  
  util::Error ToggleGIDs(const std::vector<acl::GroupID>& gids)
  {
    UnlockGuard unlock; (void) unlock;
    if (!ValidGIDs(gids)) return util::Error::Failure("Value error");
    acl::User::AddGIDs(gids);
    return util::Error::Success();
  }
  
  util::Error AddGadminGID(acl::GroupID gid)
  {
    UnlockGuard unlock; (void) unlock;
    if (gid < 0) return util::Error::Failure("Value error");
    acl::User::AddGadminGID(gid);
    return util::Error::Success();
  }
  
  util::Error DelGadminGID(acl::GroupID gid)
  {
    UnlockGuard unlock; (void) unlock;
    if (gid < 0) return util::Error::Failure("Value error");
    acl::User::DelGadminGID(gid);
    return util::Error::Success();
  }
  
  util::Error ToggleGadminGID(acl::GroupID gid)
  {
    UnlockGuard unlock; (void) unlock;
    if (gid < 0) return util::Error::Failure("Value error");
    acl::User::ToggleGadminGID(gid);
    return util::Error::Success();
  }

  util::Error SetIdleTime(int idleTime)
  {
    UnlockGuard unlock; (void) unlock;
    if (idleTime < -1) return util::Error::Failure("Value error");
    acl::User::SetIdleTime(idleTime);
    return util::Error::Success();
  }
  
  util::Error SetNumLogins(int numLogins)
  {
    UnlockGuard unlock; (void) unlock;
    if (numLogins < -1) return util::Error::Failure("Value error");
    acl::User::SetNumLogins(numLogins);
    return util::Error::Success();
  }
  
  void SetComment(const std::string& comment)
  {
    UnlockGuard unlock; (void) unlock;
    acl::User::SetComment(comment);
  }
  
  util::Error SetTagline(const std::string& tagline)
  {
    UnlockGuard unlock; (void) unlock;
    if (!acl::Validate(acl::ValidationType::Tagline, tagline)) return util::Error::Failure("Value error");
    acl::User::SetTagline(tagline);
    return util::Error::Success();
  }
  
  util::Error SetMaxDownSpeed(long long maxDownSpeed)
  {
    UnlockGuard unlock; (void) unlock;
    if (maxDownSpeed < 0) return util::Error::Failure("Value error");
    acl::User::SetMaxDownSpeed(maxDownSpeed);
    return util::Error::Success();
  }
  
  util::Error SetMaxUpSpeed(long long maxUpSpeed)
  {
    UnlockGuard unlock; (void) unlock;
    if (maxUpSpeed < 0) return util::Error::Failure("Value error");
    acl::User::SetMaxUpSpeed(maxUpSpeed);
    return util::Error::Success();
  }
  
  util::Error SetMaxSimDown(int maxSimDown)
  {
    UnlockGuard unlock; (void) unlock;
    if (maxSimDown < -1) return util::Error::Failure("Value error");
    acl::User::SetMaxSimDown(maxSimDown);
    return util::Error::Success();
  }
  
  util::Error SetMaxSimUp(int maxSimUp)
  {
    UnlockGuard unlock; (void) unlock;
    if (maxSimUp < -1) return util::Error::Failure("Value error");
    acl::User::SetMaxSimUp(maxSimUp);
    return util::Error::Success();
  }
  
  util::Error IncrSectionCredits(const std::string& section, long long kBytes)
  {
    UnlockGuard unlock; (void) unlock;
    if (kBytes < 0) return util::Error::Failure("Value error");
    acl::User::IncrSectionCredits(section, kBytes);
    return util::Error::Success();
  }
  
  util::Error DecrSectionCredits(const std::string& section, long long kBytes)
  {
    UnlockGuard unlock; (void) unlock;
    if (kBytes < 0) return util::Error::Failure("Value error");
    if (!acl::User::DecrSectionCredits(section, kBytes)) return util::Error::Failure("Not enough credits");
    return util::Error::Success();
  }
  
  util::Error DecrSectionCreditsForce(const std::string& section, long long kBytes)
  {
    UnlockGuard unlock; (void) unlock;
    if (kBytes < 0) return util::Error::Failure("Value error");
    acl::User::DecrSectionCreditsForce(section, kBytes);
    return util::Error::Success();
  }
  
  util::Error IncrDefaultCredits(long long kBytes) { return IncrSectionCredits("", kBytes); }
  util::Error DecrDefaultCredits(long long kBytes) { return DecrSectionCredits("", kBytes); }
  util::Error DecrDefaultCreditsForce(long long kBytes) { return DecrSectionCreditsForce("", kBytes); }
  
  util::Error SetSectionWeeklyAllotment(const std::string& section, long long allotment)
  {
    UnlockGuard unlock; (void) unlock;
    if (allotment < 0) return util::Error::Failure("Value error");
    acl::User::SetSectionWeeklyAllotment(section, allotment);
    return util::Error::Success();
  }
  
  util::Error SetDefaultWeeklyAllotment(long long allotment) { return SetSectionWeeklyAllotment("", allotment); }

  void Purge() const
  {
    UnlockGuard unlock; (void) unlock;
    acl::User::Purge();
  }
  
  static boost::optional<User> Load(acl::UserID uid)
  {
    UnlockGuard unlock; (void) unlock;
    auto user = acl::User::Load(uid);
    if (!user) return boost::none;
    return boost::make_optional(User(*user));
  }
  
  static boost::optional<User> Load(const std::string& name)
  {
    return Load(acl::NameToUID(name));
  }
  
  static std::pair<boost::optional<User>, std::string>
                               Create(const std::string& name, 
                                      const std::string& password, acl::UserID creator, 
                                      const User& templateUser)
  {
    UnlockGuard unlock; (void) unlock;
    if (!acl::Validate(acl::ValidationType::Username, name)) return std::make_pair(boost::none, "Value error");
    auto user = acl::User::FromTemplate(name, password, creator, templateUser);
    if (!user) return std::make_pair(boost::none, "Exists");
    return std::make_pair(boost::make_optional(User(*user)), "Success");
  }
  
  static std::vector<acl::UserID> GetUIDs(const std::string& multiStr)
  {
    UnlockGuard unlock; (void) unlock;
    return acl::User::GetUIDs(multiStr);
  }
  
  static std::vector<acl::UserID> GetUIDs() { return GetUIDs("*"); }
  
  static std::vector<User> GetUsers(const std::string& multiStr)
  {
    UnlockGuard unlock; (void) unlock;
    auto users1 = acl::User::GetUsers(multiStr);
    std::vector<User> users2;
    for (auto& user : users1)
    {
      users2.push_back(User(std::move(user)));
    }
    return users2;
  }
  
  static std::vector<User> GetUsers() { return GetUsers("*"); }
  
  static size_t TotalUsers()
  {
    UnlockGuard unlock; (void) unlock;
    return acl::User::TotalUsers();
  }

};

} /* script namespace */

#endif
