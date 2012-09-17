#ifndef __ACL_ACL_HPP
#define __ACL_ACL_HPP

#include <string>
#include <boost/ptr_container/ptr_vector.hpp>
#include "permission.hpp"

namespace acl
{

class User;

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
