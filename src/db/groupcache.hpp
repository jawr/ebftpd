#ifndef __DB_GROUPCACHE_HPP
#define __DB_GROUPCACHE_HPP

#include <string>
#include <unordered_map>
#include <boost/thread/mutex.hpp>
#include "acl/types.hpp"
#include "db/replicable.hpp"

namespace mongo
{
class BSONElement;
}

namespace db
{

struct GroupCacheBase
{
  virtual ~GroupCacheBase() { }
  virtual std::string GIDToName(acl::GroupID gid) = 0;
  virtual acl::GroupID NameToGID(const std::string& name) = 0;
};

class GroupCache : 
  public GroupCacheBase,
  public Replicable
{
  boost::mutex namesMutex;
  std::unordered_map<acl::GroupID, std::string> names;

  boost::mutex gidsMutex;
  std::unordered_map<std::string, acl::GroupID> gids;
  
public:  
  GroupCache() : Replicable("groups") { }
  std::string GIDToName(acl::GroupID gid);
  acl::GroupID NameToGID(const std::string& name);

  bool Replicate(const mongo::BSONElement& id);
  bool Populate();
};

struct GroupNoCache : public GroupCacheBase
{
  std::string GIDToName(acl::GroupID gid);
  acl::GroupID NameToGID(const std::string& name);
};

} /* db namespace */

#endif
