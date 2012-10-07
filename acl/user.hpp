#ifndef __ACL_USER_HPP
#define __ACL_USER_HPP

#include <string>
#include <unordered_set>
#include <sys/types.h>
#include "acl/flags.hpp"
#include "acl/types.hpp"

namespace db { namespace bson {
struct User;
}
}

namespace acl
{

class User
{
  std::string name;
  std::string password;
  std::string salt;
  std::string flags;

  UserID uid;
  GroupID primaryGid;
  std::unordered_set<GroupID> secondaryGids;
  
  User() : uid(-1), primaryGid(-1) { }
  
public:
  User(const std::string& name, UserID uid, const std::string& password,
       const std::string& flags);
       
  const std::string& Name() const { return name; }
  void SetName(const std::string& name) { this->name = name; }
  
  void SetPassword(const std::string& password);
  bool VerifyPassword(const std::string& password) const;
  
  const std::string& Flags() const { return flags; }
  void SetFlags(const std::string& flags) { this->flags = flags; }
  void AddFlags(const std::string& flags);
  void DelFlags(const std::string& flags);
  bool CheckFlags(const std::string& flags) const;
  bool CheckFlag(Flag flag) const;
  bool Deleted() const { return CheckFlag(Flag::Deleted); }
  
  UserID UID() const { return uid; }
  /* should never need to modify UID, can mongodb provide the id?
     possibly set this on first save to db */
  
  GroupID PrimaryGID() const { return primaryGid; }
  void SetPrimaryGID(GroupID primaryGid) { this->primaryGid = primaryGid; }
  
  const std::unordered_set<GroupID> SecondaryGIDs() const { return secondaryGids; }
  void AddSecondaryGID(GroupID gid);
  void DelSecondaryGID(GroupID gid);
  
  bool CheckGID(GroupID gid);
  
  friend struct db::bson::User;
};

}

#endif
