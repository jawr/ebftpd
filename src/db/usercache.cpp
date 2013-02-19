#include "db/usercache.hpp"
#include "db/connection.hpp"

namespace db
{

struct UserTriple
{
  std::string name;
  acl::UserID uid;
  acl::GroupID primaryGID;
};

template <> UserTriple Unserialize<UserTriple>(const mongo::BSONObj& obj)
{
  UserTriple pair;
  pair.name = obj["name"].String();
  pair.uid = obj["uid"].Int();
  pair.primaryGID = obj["primary gid"].Int();
  return pair;
}

std::string UserCache::UIDToName(acl::UserID uid)
{
  boost::lock_guard<boost::mutex> lock(namesMutex);
  auto it = names.find(uid);
  if (it == names.end()) return "unknown";
  return it->second;
}

acl::UserID UserCache::NameToUID(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(uidsMutex);
  auto it = uids.find(name);
  if (it == uids.end()) return -1;
  return it->second;
}

acl::GroupID UserCache::UIDToPrimaryGID(acl::UserID uid)
{
  boost::lock_guard<boost::mutex> lock(primaryGidsMutex);
  auto it = primaryGids.find(uid);
  if (it == primaryGids.end()) return -1;
  return it ->second;
}

bool UserCache::Replicate()
{
}

std::string UserNoCache::UIDToName(acl::UserID uid)
{
  NoErrorConnection conn;  
  auto fields = BSON("uid" << 1 << "name" << 1 << "primary gid" << 1);
  auto pair = conn.QueryOne<UserTriple>("users", QUERY("uid" << uid), &fields);
  if (!pair) return "unknown";
  return pair->name;
}

acl::UserID UserNoCache::NameToUID(const std::string& name)
{
  NoErrorConnection conn;  
  auto fields = BSON("uid" << 1 << "name" << 1 << "primary gid" << 1);
  auto pair = conn.QueryOne<UserTriple>("users", QUERY("name" << name), &fields);
  if (!pair) return -1;
  return pair->uid;
}

acl::GroupID UserNoCache::UIDToPrimaryGID(acl::UserID uid)
{
  NoErrorConnection conn;  
  auto fields = BSON("uid" << 1 << "name" << 1 << "primary gid" << 1);
  auto pair = conn.QueryOne<UserTriple>("users", QUERY("uid" << uid), &fields);
  if (!pair) return -1;
  return pair->primaryGID;
}

} /* db namespace */