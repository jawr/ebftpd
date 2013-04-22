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

#ifndef __DB_GROUPCACHE_HPP
#define __DB_GROUPCACHE_HPP

#include <string>
#include <unordered_map>
#include <mutex>
#include "acl/types.hpp"
#include "db/replicable.hpp"
#include "db/group/groupcachebase.hpp"

namespace mongo
{
class BSONElement;
}

namespace db
{

class GroupCache : 
  public GroupCacheBase,
  public Replicable
{
  std::mutex namesMutex;
  std::unordered_map<acl::GroupID, std::string> names;

  std::mutex gidsMutex;
  std::unordered_map<std::string, acl::GroupID> gids;
  
public:  
  GroupCache() : Replicable("groups") { }
  std::string GIDToName(acl::GroupID gid);
  acl::GroupID NameToGID(const std::string& name);

  bool Replicate(const mongo::BSONElement& id);
  bool Populate();
};

} /* db namespace */

#endif
