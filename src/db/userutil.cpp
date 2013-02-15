#include "db/userutil.hpp"
#include "db/connection.hpp"

namespace db
{

namespace
{
std::shared_ptr<UserCache> cache;

struct UserPair
{
  std::string name;
  acl::UserID uid;
};

UserPair Unserialize(const mongo::BSONObj& obj)
{
  UserPair pair;
  pair.name = obj["name"].String();
  pair.uid = obj["uid"].Int();
  return pair;
}

std::string LookupNameByUID(acl::UserID uid)
{
  NoErrorConnection conn;  
  auto fields = BSON("uid" << 1 << "name" << 1);
  auto pair = conn.QueryOne<UserPair>("users", QUERY("uid" << uid), &fields);
  if (!pair) return "unknown";
  return pair->name;
}

acl::UserID LookupUIDByName(const std::string& name)
{
  NoErrorConnection conn;  
  auto fields = BSON("uid" << 1 << "name" << 1);
  auto pair = conn.QueryOne<UserPair>("users", QUERY("name" << name), &fields);
  if (!pair) return -1;
  return pair->uid;
}

}

std::string UIDToName(acl::UserID uid)
{
  if (cache) return cache->UIDToName(uid);
  return LookupNameByUID(uid);
}

acl::UserID NameToUID(const std::string& name)
{
  if (cache) return cache->NameToUID(name);
  return LookupUIDByName(name);
}

} /* db namespace */
