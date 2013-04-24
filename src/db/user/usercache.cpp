//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "db/user/usercache.hpp"
#include "db/connection.hpp"
#include "util/string.hpp"
#include "db/user/user.hpp"
#include "acl/userdata.hpp"
#include "db/user/serialization.hpp"
#include "db/user/util.hpp"

namespace db
{

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

} /* db namespace */
