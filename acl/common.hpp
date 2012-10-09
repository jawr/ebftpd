#ifndef __ACL_COMMON_HPP
#define __ACL_COMMON_HPP
#include <string>
#include "unordered_set"

namespace acl
{
class ACL
{
  std::string allowFlags;
  std::string denyFlags;
  std::unordered_set<UserID> allowUID;
  std::unordered_set<UserID> denyUID;
  std::unordered_set<UserID> allowGID;
  std::unordered_set<UserID> denyGID;
public:
  ACL(const std::string& acl);
  ~ACL() {}
  // bool Check(User user);

  static ACL CreateFromString(const std::string& acl);
};
}
#endif
