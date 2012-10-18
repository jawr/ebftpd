#ifndef __DB_BSON_USERPROFILE_HPP
#define __DB_BSON_USERPROFILE_HPP

#include <mongo/client/dbclient.h>

namespace acl
{
class UserProfile;
}

namespace db { namespace bson
{

struct UserProfile
{
  static mongo::BSONObj Serialize(const acl::UserProfile& profile);
  static acl::UserProfile* Unserialize(const mongo::BSONObj& bo);
};

} /* bson namespace */
} /* db namespace */

#endif

