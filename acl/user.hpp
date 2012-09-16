#ifndef __ACL_USER_HPP
#define __ACL_USER_HPP

#include <string>
#include <boost/unordered_set.hpp>
#include "types.hpp"

namespace acl
{

class User
{
  std::string username;
  std::string password;
  std::string salt;
  std::string flags;

  uid_t uid;
  gid_t primaryGid;
  boost::unordered_set<gid_t> secondaryGids;
  
public:
  User(const std::string& username, const std::string& password,
       const std::string& flags);
       
  const std::string& Username() const { return username; }
  void Username(const std::string& username) { this->username = username; }
  
  void Password(const std::string& password);
  bool VerifyPassword(const std::string& password) const;
  
  const std::string& Flags() const { return flags; }
  void Flags(const std::string& flags) { this->flags = flags; }
  void AddFlags(const std::string& flags);
  void DelFlags(const std::string& flags);
  
  uid_t UID() const { return uid; }
  /* should never need to modify UID, can mongodb provide the id?
     possibly set this on first save to db */
  
  gid_t PrimaryGID() const { return primaryGid; }
  void PrimaryGID(gid_t primaryGid) { this->primaryGid = primaryGid; }
  
  const boost::unordered_set<gid_t> SecondaryGIDs() const { return secondaryGids; }
  void AddSecondaryGID(gid_t gid);
  void DelSecondaryGID(gid_t gid);
};

}

#endif
