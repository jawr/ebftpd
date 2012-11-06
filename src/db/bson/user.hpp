#ifndef __DB_BSON_USER_HPP
#define __DB_BSON_USER_HPP

#include <mongo/client/dbclient.h>

namespace acl
{
class User;
}

namespace db { namespace bson
{

struct User
{
  static mongo::BSONObj Serialize(const acl::User& user);
  static acl::User* UnserializePtr(const mongo::BSONObj& bo);
  static acl::User Unserialize(const mongo::BSONObj& bo);
};

} /* bson namespace */
} /* db namespace */

#endif
