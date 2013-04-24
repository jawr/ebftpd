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

#include <cassert>
#include "db/group/util.hpp"
#include "db/group/groupcache.hpp"
#include "db/group/serialization.hpp"
#include "db/connection.hpp"

namespace db
{

namespace
{

struct GroupNoCache : public GroupCacheBase
{
  std::string GIDToName(acl::GroupID gid);
  acl::GroupID NameToGID(const std::string& name);
};

std::string GroupNoCache::GIDToName(acl::GroupID gid)
{
  NoErrorConnection conn;  
  auto fields = BSON("gid" << 1 << "name" << 1 << "primary gid" << 1);
  auto data = conn.QueryOne<GroupPair>("groups", QUERY("gid" << gid), &fields);
  if (!data) return "unknown";
  return data->name;
}

acl::GroupID GroupNoCache::NameToGID(const std::string& name)
{
  NoErrorConnection conn;  
  auto fields = BSON("gid" << 1 << "name" << 1 << "primary gid" << 1);
  auto data = conn.QueryOne<GroupPair>("groups", QUERY("name" << name), &fields);
  if (!data) return -1;
  return data->gid;
}

std::shared_ptr<GroupCacheBase> groupCache(new GroupNoCache());
}

void SetGroupCache(const std::shared_ptr<GroupCacheBase>& cache)
{
  groupCache = cache;
}

std::string GIDToName(acl::GroupID gid)
{
  assert(groupCache);
  return groupCache->GIDToName(gid);
}

acl::GroupID NameToGID(const std::string& name)
{
  assert(groupCache);
  return groupCache->NameToGID(name);
}

} /* db namespace */
