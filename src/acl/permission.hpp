#ifndef __ACL_PERMISSION_HPP
#define __ACL_PERMISSION_HPP

#include <string>
#include <boost/logic/tribool.hpp>

namespace acl
{

class User;
class ACLInfo;

class Permission
{
protected:
  bool negate;

public:
  Permission(bool negate) : negate(negate) { }  
  virtual ~Permission() { }
  virtual Permission* Clone() const = 0;
  virtual boost::tribool Evaluate(const ACLInfo& info) const = 0;
  friend Permission* new_clone(const Permission& other);
};

class FlagPermission : public Permission
{
  std::string flags;

public:
  FlagPermission(const std::string& flags, bool negate) :
    Permission(negate), flags(flags)
  { }
    
  FlagPermission* Clone() const { return new FlagPermission(*this); }  
  boost::tribool Evaluate(const ACLInfo& info) const;
};

class UserPermission : public Permission
{
  std::string username;
  
public:
  UserPermission(const std::string& username, bool negate) :
    Permission(negate), username(username)
  { }
  
  UserPermission* Clone() const { return new UserPermission(*this); }
  boost::tribool Evaluate(const ACLInfo& info) const;
};

class GroupPermission : public Permission
{
  std::string groupname;
  
public:
  GroupPermission(const std::string& groupname, bool negate) :
    Permission(negate), groupname(groupname)
  { }
  
  GroupPermission* Clone() const { return new GroupPermission(*this); }
  boost::tribool Evaluate(const ACLInfo& info) const;
};

} /* acl namespace */

#endif
