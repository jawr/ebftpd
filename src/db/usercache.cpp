#include "db/usercache.hpp"
#include "db/connection.hpp"
#include "util/string.hpp"
#include "db/user.hpp"
#include "acl/userdata.hpp"

namespace db
{

std::vector<std::string> LookupIPMasks(Connection& conn, acl::UserID uid = -1)
{
  mongo::Query query;
  if (uid != -1) query = QUERY("uid" << uid);
  auto fields = BSON("ip masks" << 1);
  
  std::vector<std::string> ipMasks;
  auto results = conn.Query("users", query, 0, 0, &fields);
  for (const auto& obj : results)
  {
    try
    {
      std::vector<std::string> masks;
      UnserializeContainer(obj["ip masks"].Array(), masks);
      ipMasks.insert(ipMasks.end(), masks.begin(), masks.end());
    }
    catch (const mongo::DBException& e)
    {
      LogException("Unserialize ip masks", e, "users", query, fields);
    }
  }
  return ipMasks;
}

struct UserTriple
{
  std::string name;
  acl::UserID uid;
  acl::GroupID primaryGid;
};

template <> UserTriple Unserialize<UserTriple>(const mongo::BSONObj& obj)
{
  UserTriple data;
  data.name = obj["name"].String();
  data.uid = obj["uid"].Int();
  data.primaryGid = obj["primary gid"].Int();
  return data;
}

std::string UserCache::UIDToName(acl::UserID uid)
{
  std::lock_guard<std::mutex> lock(namesMutex);
  auto it = names.find(uid);
  if (it == names.end()) return "unknown";
  return it->second;
}

acl::UserID UserCache::NameToUID(const std::string& name)
{
  std::lock_guard<std::mutex> lock(uidsMutex);
  auto it = uids.find(name);
  if (it == uids.end()) return -1;
  return it->second;
}

acl::GroupID UserCache::UIDToPrimaryGID(acl::UserID uid)
{
  std::lock_guard<std::mutex> lock(primaryGidsMutex);
  auto it = primaryGids.find(uid);
  if (it == primaryGids.end()) return -1;
  return it ->second;
}

bool UserCache::IdentIPAllowed(const std::string& identAddress)
{
  std::lock_guard<std::mutex> lock(ipMasksMutex);
  return std::find_if(ipMasks.begin(), ipMasks.end(), 
              [&](const std::pair<acl::UserID, std::vector<std::string>>& kv)
              {
                return util::WildcardMatch(kv.second, identAddress, true);
              }) != ipMasks.end();
}

bool UserCache::IdentIPAllowed(const std::string& identAddress, acl::UserID uid)
{
  std::lock_guard<std::mutex> lock(ipMasksMutex);
  auto it = ipMasks.find(uid);
  if (it == ipMasks.end()) return false;
  return util::WildcardMatch(it->second, identAddress, true);
}

bool UserCache::Replicate(const mongo::BSONElement& id)
{
  if (id.type() != 16) return true;
  acl::UserID uid = id.Int();

  updatedCallback(uid);
  
  try
  {
    SafeConnection conn;  
    auto fields = BSON("uid" << 1 << "name" << 1 << "primary gid" << 1);
    auto data = conn.QueryOne<UserTriple>("users", QUERY("uid" << uid), &fields);
    if (data)
    {
      // user found, refresh cached data
      {
        std::lock_guard<std::mutex> lock(uidsMutex);
        uids[data->name] = data->uid;
      }
      
      {
        std::lock_guard<std::mutex> lock(namesMutex);
        names[data->uid] = data->name;
      }
      
      {
        std::lock_guard<std::mutex> lock(primaryGidsMutex);
        primaryGids[data->uid] = data->primaryGid;
      }
      
      auto masks = LookupIPMasks(conn, uid);
      
      {
        std::lock_guard<std::mutex> lock(ipMasksMutex);
        ipMasks[data->uid] = std::move(masks);
      }
    }
    else
    {
      // user not found, must be deleted, remove from cache
      {
        std::lock(uidsMutex, namesMutex);
        std::lock_guard<std::mutex> uidsLock(uidsMutex, std::adopt_lock);
        std::lock_guard<std::mutex> namesLock(namesMutex, std::adopt_lock);
        
        auto it = names.find(uid);
        if (it != names.end())
        {
          uids.erase(it->second);
          names.erase(it);
        }
      }
      
      {
        std::lock_guard<std::mutex> lock(primaryGidsMutex);
        primaryGids.erase(uid);
      }
      
      {
        std::lock_guard<std::mutex> lock(ipMasksMutex);
        ipMasks.erase(uid);
      }
    }
  }
  catch (const DBError&)
  {
    return false;
  }
  
  return true;
}

bool UserCache::Populate()
{
  auto users = GetUsers();
  
  std::lock(namesMutex, uidsMutex, primaryGidsMutex, ipMasksMutex);
  std::lock_guard<std::mutex> namesLock(namesMutex, std::adopt_lock);
  std::lock_guard<std::mutex> uidsLock(uidsMutex, std::adopt_lock);
  std::lock_guard<std::mutex> primaryGidsLock(primaryGidsMutex, std::adopt_lock);
  std::lock_guard<std::mutex> ipMasksLock(ipMasksMutex, std::adopt_lock);
  
  uids.clear();
  names.clear();
  primaryGids.clear();
  ipMasks.clear();
  
  for (const auto& user : users)
  {
    uids[user.name] = user.id;
    names[user.id] = std::move(user.name);
    primaryGids[user.id] = user.primaryGid;
    ipMasks[user.id] = std::move(user.ipMasks);
  }
  
  return true;
}

std::string UserNoCache::UIDToName(acl::UserID uid)
{
  NoErrorConnection conn;  
  auto fields = BSON("uid" << 1 << "name" << 1 << "primary gid" << 1);
  auto data = conn.QueryOne<UserTriple>("users", QUERY("uid" << uid), &fields);
  if (!data) return "unknown";
  return data->name;
}

acl::UserID UserNoCache::NameToUID(const std::string& name)
{
  NoErrorConnection conn;  
  auto fields = BSON("uid" << 1 << "name" << 1 << "primary gid" << 1);
  auto data = conn.QueryOne<UserTriple>("users", QUERY("name" << name), &fields);
  if (!data) return -1;
  return data->uid;
}

acl::GroupID UserNoCache::UIDToPrimaryGID(acl::UserID uid)
{
  NoErrorConnection conn;  
  auto fields = BSON("uid" << 1 << "name" << 1 << "primary gid" << 1);
  auto data = conn.QueryOne<UserTriple>("users", QUERY("uid" << uid), &fields);
  if (!data) return -1;
  return data->primaryGid;
}

bool UserNoCache::IdentIPAllowed(const std::string& identAddress)
{
  NoErrorConnection conn;
  return util::WildcardMatch(LookupIPMasks(conn), identAddress, true);
}

bool UserNoCache::IdentIPAllowed(const std::string& identAddress, acl::UserID uid)
{
  NoErrorConnection conn;
  return util::WildcardMatch(LookupIPMasks(conn, uid), identAddress, true);
}

} /* db namespace */