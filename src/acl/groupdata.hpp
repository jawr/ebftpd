#ifndef __ACL_GROUPDATA_HPP
#define __ACL_GROUPDATA_HPP

#include <string>
#include "acl/types.hpp"

namespace acl
{

struct GroupData
{
  acl::GroupID id;
  std::string name;

  std::string description;
  std::string comment;
  
  int slots;
  int leechSlots;
  int allotmentSlots;
  long long maxAllotmentSize;
  int maxLogins;

  GroupData() :
    id(-1),
    slots(0),
    leechSlots(-2),
    allotmentSlots(-2),
    maxAllotmentSize(0),
    maxLogins(-1)
  {
  }
};

} /* acl namespace */

#endif
