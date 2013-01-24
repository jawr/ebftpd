#ifndef __ACL_USER_HPP
#define __ACL_USER_HPP

#include <string>
#include <vector>
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
  boost::posix_time::ptime modified;

  std::string name;
  std::string password;
  std::string salt;
  std::string flags;

  UserID uid;
  GroupID primaryGid;
  std::vector<GroupID> secondaryGids;
  
  long long credits;
  std::string tagline;
   
public:
  User() :
    modified(boost::posix_time::microsec_clock::local_time()),
    uid(-1),
    primaryGid(-1),
    credits(0)
  { }
  
  User(const std::string& name, UserID uid, const std::string& password,
       const std::string& flags);

  const boost::posix_time::ptime& Modified() const { return modified; }
       
  const std::string& Name() const { return name; }
  void SetName(const std::string& name)
  {
    modified = boost::posix_time::microsec_clock::local_time();
    this->name = name;
  }

  long long Credits() const { return credits; }
  
  void DecrCredits(long long kbytes)
  {
    modified = boost::posix_time::microsec_clock::local_time();
    credits -= kbytes;
  }
  
  void IncrCredits(long long kbytes)
  {
    modified = boost::posix_time::microsec_clock::local_time();
    credits += kbytes;
  }
  
  void SetPassword(const std::string& password);
  bool VerifyPassword(const std::string& password) const;
  
  const std::string& Flags() const { return flags; }
  void SetFlags(const std::string& flags);
  void AddFlags(const std::string& flags);
  void AddFlag(Flag flag);
  void DelFlags(const std::string& flags);
  void DelFlag(Flag flag);
  bool CheckFlags(const std::string& flags) const;
  bool CheckFlag(Flag flag) const;
  bool Deleted() const { return CheckFlag(Flag::Deleted); }
  bool Template() const { return CheckFlag(Flag::Template); }
  
  UserID UID() const { return uid; }
  
  GroupID PrimaryGID() const { return primaryGid; }
  void SetPrimaryGID(GroupID primaryGid);
  
  const std::vector<GroupID> SecondaryGIDs() const { return secondaryGids; }
  void AddSecondaryGID(GroupID gid);
  void DelSecondaryGID(GroupID gid);
  void ResetSecondaryGIDs();
  bool HasSecondaryGID(GroupID gid);
  
  bool CheckGID(GroupID gid);
  
  const std::string& Tagline() const { return tagline; }
  void SetTagline(const std::string& tagline) { this->tagline = tagline; }
  
  friend struct db::bson::User;
};

}

#endif
