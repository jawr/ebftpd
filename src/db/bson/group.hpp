#ifndef __DB_BSON_GROUP_HPP
#define __DB_BSON_GROUP_HPP

#include <memory>

namespace mongo
{
class BSONObj;
}

namespace acl
{
class Group;
}

namespace db { namespace bson
{

struct Group
{
  static mongo::BSONObj Serialize(const acl::Group& group);
  static std::unique_ptr<acl::Group> UnserializePtr(const mongo::BSONObj& bo);
  static acl::Group Unserialize(const mongo::BSONObj& bo);
  static void Unserialize(const mongo::BSONObj& bo, acl::Group& group);
};

} /* bson namespace */
} /* db namespace */

#endif
