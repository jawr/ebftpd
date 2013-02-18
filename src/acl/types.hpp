#ifndef __ACL_TYPES_HPP
#define __ACL_TYPES_HPP

#include <cstdint>

namespace acl
{

/*class GroupID;

class UserID
{
  int32_t id;

public:
  UserID() : id(-1) { }
  UserID(int32_t id) : id(id) { }
  operator int32_t() const { return id; }
};

class GroupID
{
  int32_t id;

public:
  GroupID() : id(-1) { }
  GroupID(int32_t id) : id(id) { }
  operator int32_t() const { return id; }
};
*/
typedef int32_t UserID;
typedef int32_t GroupID;

} /* acl namespace */

#endif
