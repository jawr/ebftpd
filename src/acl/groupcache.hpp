#ifndef __ACL_GROUPCACHE_HPP
#define __ACL_GROUPCACHE_HPP

#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <boost/thread/mutex.hpp>
#include "acl/group.hpp"
#include "acl/types.hpp"
#include "util/error.hpp"

namespace acl
{

class GroupCache
{
  typedef std::unordered_map<std::string, acl::Group*> ByNameMap;
  typedef std::unordered_map<GroupID, acl::Group*> ByGIDMap;
  
  mutable boost::mutex mutex;
  ByNameMap byName;
  ByGIDMap byGID;
  
  static GroupCache instance;
  static bool initialized;
  
  ~GroupCache();
  
  static void Save(const acl::Group& group);
  
public:
  static void Initialize();
  static bool Initialized() { return instance.initialized; }

  static bool Exists(const std::string& name);
  static bool Exists(GroupID gid);
  static util::Error Create(const std::string& name);
  static util::Error Delete(const std::string& name);
  static util::Error Rename(const std::string& oldName, const std::string& newName);
  
  // these return const as the group objects should NEVER
  // be modified except via the above functions'
  static acl::Group Group(const std::string& name);
  static acl::Group Group(GroupID gid);
  
  static GroupID NameToGID(const std::string& name);
  static std::string GIDToName(GroupID gid);
};

} /* acl namespace */

#endif
