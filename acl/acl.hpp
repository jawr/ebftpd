#ifndef __ACL_ACL_HPP
#define __ACL_ACL_HPP

#include <string>
#include <boost/unordered_set.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "types.hpp"

namespace acl
{

class User;

class Permission
{
friend Permission* new_clone(Permission const& other);
protected:
  bool negate;

  virtual Permission* Clone() const = 0;
  
public:
  Permission(bool negate) : negate(negate) { }
  
  virtual boost::tribool Evaluate(const User& user) const = 0;
};

class FlagPermission : public Permission
{
  std::string flags;

  FlagPermission* Clone() const { return new FlagPermission(*this); }
  
public:
  FlagPermission(const std::string& flags, bool negate) :
    Permission(negate), flags(flags)
  {
  }
    
  boost::tribool Evaluate(const User& user) const;
};

class UserPermission : public Permission
{
  std::string username;
  
  UserPermission* Clone() const { return new UserPermission(*this); }

public:
  UserPermission(const std::string& username, bool negate) :
    Permission(negate), username(username)
  {
  }
  
  boost::tribool Evaluate(const User& user) const;
};

class GroupPermission : public Permission
{
  std::string groupname;
  
  GroupPermission* Clone() const { return new GroupPermission(*this); }

public:
  GroupPermission(const std::string& groupname, bool negate) :
    Permission(negate), groupname(groupname)
  {
  }
  
  boost::tribool Evaluate(const User& user) const;
};

class ACL
{
  boost::ptr_vector<Permission> perms;

  ACL() { }   
  void FromStringArg(const std::string& arg);
  
public:
  bool Check(const User& user);
  static ACL FromString(const std::string& str);
};

} /* acl namespace */

#endif
