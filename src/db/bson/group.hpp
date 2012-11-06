#ifndef __DB_BSON_GROUP_HPP
#define __DB_BSON_GROUP_HPP

#include <memory>
#include <mongo/client/dbclient.h>

namespace acl
{
class Group;
}

namespace db { namespace bson
{

struct Group
{
  static mongo::BSONObj Serialize(const acl::Group& group);
  static std::unique_ptr<acl::Group> Unserialize(const mongo::BSONObj& bo);
};

} /* bson namespace */
} /* db namespace */

#endif
