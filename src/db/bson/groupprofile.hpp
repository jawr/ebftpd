#ifndef __DB_BSON_GROUPPROFILE_HPP
#define __DB_BSON_GROUPPROFILE_HPP

namespace mongo
{
class BSONObj;
}

namespace acl
{
class GroupProfile;
}

namespace db { namespace bson
{

struct GroupProfile
{
  static mongo::BSONObj Serialize(const acl::GroupProfile& profile);
  static acl::GroupProfile Unserialize(const mongo::BSONObj& bo);
};

} /* bson namespace */
} /* db namespace */

#endif
