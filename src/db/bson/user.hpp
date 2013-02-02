#ifndef __DB_BSON_USER_HPP
#define __DB_BSON_USER_HPP

namespace mongo
{
class BSONObj;
}

namespace acl
{
class User;
}

namespace db { namespace bson
{

struct User
{
  static mongo::BSONObj Serialize(const acl::User& user);
  static void Unserialize(const mongo::BSONObj& bo, acl::User& user);
  static std::unique_ptr<acl::User> UnserializePtr(const mongo::BSONObj& bo);
  static acl::User Unserialize(const mongo::BSONObj& bo);
};

} /* bson namespace */
} /* db namespace */

#endif
