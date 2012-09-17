#ifndef __ACL_COMMON_HPP
#define __ACL_COMMON_HPP
#include <string>
#include "boost/unordered_set.hpp"

namespace acl
{
class ACL
{
  std::string allowFlags;
  std::string denyFlags;
  boost::unordered_set<uid_t> allowUID;
  boost::unordered_set<uid_t> denyUID;
  boost::unordered_set<uid_t> allowGID;
  boost::unordered_set<uid_t> denyGID;
public:
  ACL(const std::string& acl);
  ~ACL() {};
  // bool Check(User user);

  static ACL CreateFromString(const std::string& acl);
};
}
#endif
