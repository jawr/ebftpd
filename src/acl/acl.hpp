#ifndef __ACL_ACL_HPP
#define __ACL_ACL_HPP

#include <string>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/optional.hpp>
#include "acl/permission.hpp"

namespace acl
{

class User;

struct ACLInfo
{
  std::string username;
  std::string groupname;
  std::string flags;
  
  ACLInfo(const std::string& username, 
          const std::string& groupname, 
          const std::string& flags) :
    username(username),
    groupname(groupname),
    flags(flags)
  {
  }
};

class ACL
{
  boost::ptr_vector<Permission> perms;
  mutable boost::optional<bool> finalResult;

  void FromStringArg(const std::string& arg);
  void FromString(const std::string& str);
  
public:
  ACL() = default;
  ACL(const std::string& s);
 
  bool Evaluate(const ACLInfo& info) const;

  bool Evaluate(const std::string& username, 
                const std::string& groupname, 
                const std::string& flags) const
  {
    return Evaluate(ACLInfo(username, groupname, flags));
  }
};

} /* acl namespace */

#endif
