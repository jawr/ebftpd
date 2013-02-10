#ifndef __ACL_USER_HPP
#define __ACL_USER_HPP

#include <string>
#include <vector>
#include <sys/types.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "acl/flags.hpp"
#include "acl/types.hpp"
#include "util/error.hpp"

namespace acl { class User; }
namespace db { namespace bson { struct User; }
namespace user { bool Create(acl::User& user); } }

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
  std::vector<GroupID> gadminGids;
  
  std::vector<std::string> ipMasks;
   
public:
  User() :
    modified(boost::posix_time::microsec_clock::local_time()),
    flags("6"),
    uid(-1),
    primaryGid(-1)
  { }
  
  User(const std::string& name, const std::string& password, const std::string& flags);

  const boost::posix_time::ptime& Modified() const { return modified; }
       
  const std::string& Name() const { return name; }
  void SetName(const std::string& name)
  {
    modified = boost::posix_time::microsec_clock::local_time();
    this->name = name;
  }

  void SetPassword(const std::string& password);
  bool VerifyPassword(const std::string& password) const;
  
  const std::string& Flags() const { return flags; }
  void SetFlags(const std::string& flags);
  void AddFlags(const std::string& flags);
  void AddFlag(Flag flag);
  void DelFlags(const std::string& flags);
  void DelFlag(Flag flag);
  bool CheckFlags(const std::string& flags) const
  {
    for (char ch: flags)
    {
      if (this->flags.find(ch) != std::string::npos) return true;
    }
    return false;
  }
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
  bool HasSecondaryGID(GroupID gid) const;
  bool HasGID(GroupID gid) const;
  
  const std::vector<GroupID> GadminGIDs() const { return gadminGids; }
  void AddGadminGID(GroupID gid);
  void DelGadminGID(GroupID gid);
  bool HasGadminGID(GroupID gid) const;
  
  
  util::Error AddIPMask(const std::string& mask, std::vector<std::string> &redundant);
  util::Error DelIPMask(decltype(ipMasks.size()) index, std::string& deleted);
  void DelAllIPMasks(std::vector<std::string>& deleted);
  std::vector<std::string> ListIPMasks() const
  { return ipMasks; }
  
  friend struct db::bson::User;
  friend bool db::user::Create(acl::User& user);
};

}

#endif
