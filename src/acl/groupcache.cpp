#include <memory>
#include "acl/groupcache.hpp"
#include "db/group/group.hpp"
#include "logs/logs.hpp"

namespace acl
{

GroupCache GroupCache::instance;
bool GroupCache::initalized = false;

GroupCache::~GroupCache()
{
  while (!byName.empty())
  {
    delete byName.begin()->second;
    byName.erase(byName.begin());
  }
}

void GroupCache::Initalize()
{
  if (instance.initalized) return;
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  boost::ptr_vector<acl::Group> groups;
  db::group::GetAll(groups);
  while (!groups.empty())
  {
    auto group = groups.release(groups.begin());
  
    instance.byGID.insert(std::make_pair(group->GID(), group.get()));
    instance.byName.insert(std::make_pair(group->Name(), group.get()));
    group.release();
  } 

  instance.initalized = true;
}

void GroupCache::Save(const acl::Group& group)
{
  db::group::Save(group);
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
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  if (instance.byName.find(name) != instance.byName.end())
    return util::Error::Failure("Group already exists");

  acl::GroupID gid = db::group::GetNewGroupID();

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

  db::group::Delete(it->second->GID());

  instance.byGID.erase(instance.byGID.find(it->second->GID()));
  delete it->second;
  instance.byName.erase(it);
    
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

acl::Group GroupCache::Group(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) throw util::RuntimeError("Group doesn't exist");
  return *it->second;
}

acl::Group GroupCache::Group(GroupID gid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByGIDMap::iterator it = instance.byGID.find(gid);
  if (it == instance.byGID.end()) throw util::RuntimeError("Group doesn't exist");
  return *it->second;
}

GroupID GroupCache::NameToGID(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return -1;
  return it->second->GID();
}

std::string GroupCache::GIDToName(GroupID gid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByGIDMap::iterator it = instance.byGID.find(gid);
  if (it == instance.byGID.end()) return "unknown";
  return it->second->Name();
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
