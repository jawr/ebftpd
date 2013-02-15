#include "db/grouputil.hpp"
#include "db/connection.hpp"

namespace db
{

namespace
{
std::shared_ptr<GroupCache> cache;

struct GroupPair
{
  std::string name;
  acl::GroupID gid;
};

GroupPair Unserialize(const mongo::BSONObj& obj)
{
  GroupPair pair;
  pair.name = obj["name"].String();
  pair.gid = obj["gid"].Int();
  return pair;
}

std::string LookupNameByGID(acl::GroupID gid)
{
  NoErrorConnection conn;  
  auto fields = BSON("gid" << 1 << "name" << 1);
  auto pair = conn.QueryOne<GroupPair>("groups", QUERY("gid" << gid), &fields);
  if (!pair) return "unknown";
  return pair->name;
}

acl::GroupID LookupGIDByName(const std::string& name)
{
  NoErrorConnection conn;  
  auto fields = BSON("gid" << 1 << "name" << 1);
  auto pair = conn.QueryOne<GroupPair>("groups", QUERY("name" << name), &fields);
  if (!pair) return -1;
  return pair->gid;
}

}

std::string GIDToName(acl::GroupID gid)
{
  if (cache) return cache->GIDToName(gid);
  return LookupNameByGID(gid);
}

acl::GroupID NameToGID(const std::string& name)
{
  if (cache) return cache->NameToGID(name);
  return LookupGIDByName(name);
}

} /* db namespace */
