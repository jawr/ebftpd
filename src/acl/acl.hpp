#ifndef __ACL_ACL_HPP
#define __ACL_ACL_HPP

#include <string>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/optional.hpp>
#include "acl/permission.hpp"

namespace acl
{

class User;

class ACL
{
  boost::ptr_vector<Permission> perms;
  mutable boost::optional<bool> finalResult;

  void FromStringArg(const std::string& arg);
  
public:
  bool Evaluate(const User& user) const;
  static ACL FromString(const std::string& str);
};

} /* acl namespace */

#endif
