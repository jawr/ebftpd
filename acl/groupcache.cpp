#include <memory>
#include "acl/groupcache.hpp"

// only for generating dodgy random gid
// until we can retrieve one from db
#include <ctime>
#include <cstdlib>

namespace acl
{

GroupCache GroupCache::instance;

GroupCache::~GroupCache()
{
  while (!byName.empty())
  {
    delete byName.begin()->second;
    byName.erase(byName.begin());
  }
}

void GroupCache::Save(const acl::Group& group)
{
  // create task holding a copy of group for db connection pool to 
  // save into database
}

bool GroupCache::Exists(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byName.find(name) != instance.byName.end();
}

bool GroupCache::Exists(GroupID gid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byGID.find(gid) != instance.byGID.end();
}

util::Error GroupCache::Create(const std::string& name)
{
  // somewhere here we need to query the db connection pool
  // for a new gid  
  
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  if (instance.byName.find(name) != instance.byName.end())
    return util::Error::Failure("Group already exists");

  // dummy random gid
  GroupID gid;
  while (true)
  {
    gid = rand() % 10000 + 1; /* 1 to 10000 */
    if (instance.byGID.find(gid) == instance.byGID.end())
      break;
  }

  std::unique_ptr<acl::Group> group(new acl::Group(name, gid));
    
  instance.byName.insert(std::make_pair(name, group.get()));
  instance.byGID.insert(std::make_pair(gid, group.get()));
  
  Save(*group.release());
  
  return util::Error::Success();
}

util::Error GroupCache::Delete(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("Group doesn't exist");

  instance.byGID.erase(instance.byGID.find(it->second->GID()));
  delete it->second;
  instance.byName.erase(it);
  
  // create a task for the db connection pool to execute
  // telling it to delete this group frmo database
  
  return util::Error::Success();
}

util::Error GroupCache::Rename(const std::string& oldName, const std::string& newName)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  if (instance.byName.find(newName) != instance.byName.end())
    return util::Error::Failure("Group already exists with new name");
  
  ByNameMap::iterator it = instance.byName.find(oldName);
  if (it == instance.byName.end()) return util::Error::Failure("Group doesn't exist");

  it->second->SetName(newName);
  instance.byName.insert(std::make_pair(newName, it->second));
  instance.byName.erase(it);
  
  Save(*it->second);
  
  return util::Error::Success();
}

const acl::Group GroupCache::Group(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) throw util::RuntimeError("Group doesn't exist");
  return *it->second;
}

const acl::Group GroupCache::Group(GroupID gid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByGIDMap::iterator it = instance.byGID.find(gid);
  if (it == instance.byGID.end()) throw util::RuntimeError("Group doesn't exist");
  return *it->second;
}

} /* acl namespace */


#ifdef ACL_GROUPCACHE_TEST

int main()
{
  using namespace acl;
  
  std::cout << "exists: " << GroupCache::Exists("somegroup") << std::endl;
  std::cout << "exists: " << GroupCache::Exists(123) << std::endl;
  
  std::cout << "create: " << GroupCache::Create("somegroup") << std::endl;
  std::cout << "exists: " << GroupCache::Exists("somegroup") << std::endl;
  
  Group group = GroupCache::Group("somegroup");
  std::cout << "exists: " << GroupCache::Exists(group.GID()) << std::endl;
  std::cout << "rename: " << GroupCache::Rename("somegroup" ,"othergroup") << std::endl;
  std::cout << "exists: " << GroupCache::Exists("othergroup") << std::endl;
  std::cout << "exists: " << GroupCache::Exists(group.GID()) << std::endl;
  std::cout << "exists: " << GroupCache::Exists("somegroup") << std::endl;
  GroupCache::Delete("othergroup");
  std::cout << "exists: " << GroupCache::Exists("othergroup") << std::endl;
  std::cout << "exists: " << GroupCache::Exists(group.GID()) << std::endl;
  
  
}

#endif
