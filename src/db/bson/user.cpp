#include <vector>
#include <memory>
#include "db/bson/user.hpp"
#include "acl/user.hpp"
#include "db/bson/bson.hpp"
#include "logs/logs.hpp"

namespace db { namespace bson
{

mongo::BSONObj User::Serialize(const acl::User& user)
{
  mongo::BSONObjBuilder bob;
  bob.append("name", user.name);
  bob.append("salt", user.salt);
  bob.append("password", user.password);
  bob.append("flags", user.flags);
  bob.append("uid", user.uid);
  bob.append("credits", user.credits);
  bob.append("primary gid", user.primaryGid);
  bob.append("secondary gids", SerializeContainer(user.secondaryGids));
  return bob.obj();
}

acl::User* User::UnserializePtr(const mongo::BSONObj& bo)
{
  std::unique_ptr<acl::User> user(new acl::User);
  user->name = bo["name"].String();
  user->salt = bo["salt"].String();
  user->credits = bo["credits"].Long();
  user->password = bo["password"].String();
  user->flags = bo["flags"].String();
  user->uid = bo["uid"].Int();
  user->primaryGid = bo["primary gid"].Int();
  std::vector<mongo::BSONElement> secondaryGids = bo["secondary gids"].Array();
  for (const auto& el: secondaryGids)
    user->secondaryGids.insert(el.Int());

  return user.release();
}

acl::User User::Unserialize(const mongo::BSONObj& bo)
{
  acl::User user;
  user.name = bo["name"].String();
  user.salt = bo["salt"].String();
  user.credits = bo["credits"].Long();
  user.password = bo["password"].String();
  user.flags = bo["flags"].String();
  user.uid = bo["uid"].Int();
  user.primaryGid = bo["primary gid"].Int();
  std::vector<mongo::BSONElement> secondaryGids = bo["secondary gids"].Array();
  for (const auto& el: secondaryGids)
    user.secondaryGids.insert(el.Int());

  return user;
}
  
} /* bson namespace */
} /* db namespace */


#ifdef DB_BSON_USER_TEST

#include <iostream>

int main()
{
  using namespace db::bson;
  using namespace mongo;
  
  acl::User user("someone", 123, "password", "1");
  
  BSONObj bo = User::Serialize(user);
  std::cout << bo["name"].String() << " " << bo["uid"].Int() << std::endl;
  user = User::Unserialize(bo);
  std::cout << user.Name() << " " << user.UID() << std::endl;
  
}

#endif
