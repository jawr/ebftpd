#ifndef __ACL_TYPES_HPP
#define __ACL_TYPES_HPP

#include <boost/cstdint.hpp>
#include <map>
#include <vector>
#include <string>

namespace acl
{

typedef boost::int32_t UserID;
typedef boost::int32_t GroupID;

typedef std::unordered_map<acl::UserID, std::vector<std::string>> UserMaskMap;

} /* acl namespace */

#endif
