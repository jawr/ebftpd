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

#include "db/group/groupcache.hpp"
#include "db/connection.hpp"
#include "util/string.hpp"
#include "db/group/group.hpp"
#include "acl/groupdata.hpp"
#include "db/group/serialization.hpp"

namespace db
{

std::string GroupCache::GIDToName(acl::GroupID gid)
{
  if (gid == -1) return "NoGroup";
  std::lock_guard<std::mutex> lock(namesMutex);
  auto it = names.find(gid);
  if (it == names.end()) return "unknown";
  return it->second;
}

acl::GroupID GroupCache::NameToGID(const std::string& name)
{
  std::lock_guard<std::mutex> lock(gidsMutex);
  auto it = gids.find(name);
  if (it == gids.end()) return -1;
  return it->second;
}

bool GroupCache::Replicate(const mongo::BSONElement& id)
{
  if (id.type() != 16) return true;
  acl::GroupID gid = id.Int();

  try
  {
    SafeConnection conn;  
    auto fields = BSON("gid" << 1 << "name" << 1);
    auto data = conn.QueryOne<GroupPair>("groups", QUERY("gid" << gid), &fields);
    if (data)
    {
      // group found, refresh cached data
      {
        std::lock_guard<std::mutex> lock(gidsMutex);
        gids[data->name] = data->gid;
      }
      
      {
        std::lock_guard<std::mutex> lock(namesMutex);
        names[data->gid] = data->name;
      }
    }
    else
    {
      // group not found, must be deleted, remove from cache
      std::lock(gidsMutex, namesMutex);
      std::lock_guard<std::mutex> gidsLock(gidsMutex, std::adopt_lock);
      std::lock_guard<std::mutex> namesLock(namesMutex, std::adopt_lock);
      
      auto it = names.find(gid);
      if (it != names.end())
      {
        gids.erase(it->second);
        names.erase(it);
      }
    }
  }
  catch (const DBError&)
  {
    return false;
  }
  
  return true;
}

bool GroupCache::Populate()
{
  auto groups = GetGroups();
  
  std::lock(namesMutex, gidsMutex);
  std::lock_guard<std::mutex> namesLock(namesMutex, std::adopt_lock);
  std::lock_guard<std::mutex> gidsLock(gidsMutex, std::adopt_lock);

  gids.clear();
  names.clear();
  
  for (const auto& group : groups)
  {
    gids[group.name] = group.id;
    names[group.id] = group.name;
  }

  return true;
}

} /* db namespace */
