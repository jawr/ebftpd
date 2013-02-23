#ifndef __ACL_ACL_HPP
#define __ACL_ACL_HPP

#include <string>
#include <vector>
#include <boost/optional.hpp>
#include "acl/permission.hpp"

namespace acl
{

class User;

class ACL
{
  std::vector<Permission*> perms;
  mutable boost::optional<bool> finalResult;

  void FromStringArg(const std::string& arg);
  
public:
  ACL() = default;
  ACL& operator=(const ACL& rhs);
  ACL& operator=(ACL&& rhs);
  ACL(const ACL& other);
  ACL(ACL&& other);
  ~ACL();

  bool Evaluate(const User& user) const;
  static ACL FromString(const std::string& str);
};

} /* acl namespace */

#endif
