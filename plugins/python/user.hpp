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

#ifndef __SCRIPTING_PYTHON_USER_HPP
#define __SCRIPTING_PYTHON_USER_HPP

#include <stdexcept>
#include <boost/optional.hpp>
#include "acl/user.hpp"

namespace scripting { namespace python
{

struct NoUnlocking
{
  static void Unlock() { }
  static void Lock() { }
};

struct PythonReleaseGIL
{
  static void Unlock() { /* unlock GIL */ }
  static void Lock() { /* lock GIL */ }
  
  PythonReleaseGIL()
  {
    Unlock();
  }
  
  ~PythonReleaseGIL()
  {
    Lock();
  }
};

struct NotConstructable : std::runtime_error
{
  NotConstructable() : std::runtime_error("Not constructible") { }
};

template <typename UnlockingPolicy = NoUnlocking>
class User : public acl::User
{
  User(acl::User&& user)
  {
  }
  
public:
  User()
  {
    throw NotConstructible();
  }

/*
  bool Rename(const std::string& name);
  bool AddIPMask(const std::string& ipMask, std::vector<std::string>* deleted);  
  void DelIPMask(const std::string& ipMask);
  std::string DelIPMask(size_t index);
  void ClearIPMasks();

  void SetFlags(const std::string& flags);
  void AddFlags(const std::string& flags);
  void DelFlags(const std::string& flags);

  void SetPrimaryGID(acl::GroupID gid);
  void AddGIDs(const std::vector<acl::GroupID>& gids);
  void DelGIDs(const std::vector<acl::GroupID>& gids);
  void SetGIDs(const std::vector<acl::GroupID>& gids);
  void ToggleGIDs(const std::vector<acl::GroupID>& gids);

  void AddGadminGID(acl::GroupID gid);
  void DelGadminGID(acl::GroupID gid);
  bool ToggleGadminGID(acl::GroupID gid);

  void SetHomeDir(const std::string& homeDir);
  
  void SetIdleTime(int idleTime);
  
  void SetExpires(const boost::optional<boost::gregorian::date>& expires);
  
  void SetNumLogins(int numLogins);
  
  void SetComment(const std::string& comment);
  
  void SetTagline(const std::string& tagline);
  
  void SetMaxDownSpeed(long long maxDownSpeed);
  
  void SetMaxUpSpeed(long long maxUpSpeed);
  
  void SetMaxSimDown(int maxSimDown);
  
  void SetMaxSimUp(int maxSimUp);
  
  void SetLoggedIn();
  
  void SetSectionRatio(const std::string& section, int ratio);
  
  void IncrSectionCredits(const std::string& section, long long kBytes);
  bool DecrSectionCredits(const std::string& section, long long kBytes);
  void DecrSectionCreditsForce(const std::string& section, long long kBytes);
  

  void SetSectionWeeklyAllotment(const std::string& section, long long allotment)
*/
  void Purge() const
  {
    UnlockingPolicy unlock; (void) unlock;
    acl::User::Purge();
  }
  
/*  static boost::optional<User> Load(acl::UserID uid)
  {
    UnlockingPolicy unlock; (void) unlock;
    auto user = acl::User::Load(uid);
    if (!user) return boost::none;
    return make_optional(User(*user));
  }
  
  static boost::optional<User> Load(const std::string& name)
  {
    return Load(acl::NameToUID(name));
  }
  
  static std::vector<acl::UserID> GetUIDs(const std::string& multiStr)
  {
    UnlockingPolicy unlock; (void) unlock;
    return acl::User::GetUIDs(multiStr);
  }
  
  static std::vector<acl::UserID> GetUIDs() { return GetUIDs("*"); }
  
  static std::vector<User> GetUsers(const std::string& multiStr)
  {
    UnlockingPolicy unlock; (void) unlock;
    auto users1 = acl::User::GetUIDs(multiStr);
    std::vector<User> users2;
    for (auto& user : users1)
    {
      users2.emplace_back(user);
    }
    return users2;
  }
  
  static std::vector<User> GetUsers() { return GetUsers("*"); }*/
};

} /* python namespace */
} /* scripting namespace */

#endif
