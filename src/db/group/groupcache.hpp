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
