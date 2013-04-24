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
#include "util/string.hpp"
#include "db/user/util.hpp"
#include "db/user/usercache.hpp"
#include "db/group/group.hpp"
#include "acl/user.hpp"
#include "db/connection.hpp"
#include "db/user/usercachebase.hpp"
#include "db/user/serialization.hpp"

namespace db
{

namespace
{

struct UserNoCache : public UserCacheBase
{
  std::string UIDToName(acl::UserID uid);
  acl::UserID NameToUID(const std::string& name);
  acl::GroupID UIDToPrimaryGID(acl::UserID uid);  
  bool IdentIPAllowed(const std::string& identAddress);
  bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid);
};

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

std::shared_ptr<UserCacheBase> userCache(new UserNoCache());
}

void SetUserCache(const std::shared_ptr<UserCacheBase>& cache)
{
  userCache = cache;
}

std::string UIDToName(acl::UserID uid)
{
  assert(userCache);
  return userCache->UIDToName(uid);
}

acl::UserID NameToUID(const std::string& name)
{
  assert(userCache);
  return userCache->NameToUID(name);
}

acl::GroupID UIDToPrimaryGID(acl::UserID uid)
{
  assert(userCache);
  return userCache->UIDToPrimaryGID(uid);
}

bool IdentIPAllowed(const std::string& identAddress)
{
  assert(userCache);
  return userCache->IdentIPAllowed(identAddress);
}

bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid)
{
  assert(userCache);
  return userCache->IdentIPAllowed(identAddress, uid);
}

std::vector<std::string> LookupIPMasks(Connection& conn, acl::UserID uid)
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

} /* db namespace */