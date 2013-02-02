#include <memory>
#include "acl/groupcache.hpp"
#include "db/group/group.hpp"
#include "logs/logs.hpp"
#include "db/group/groupprofile.hpp"
#include "acl/replicator.hpp"
#include "acl/groupprofile.hpp"

namespace acl
{

GroupCache GroupCache::instance;
bool GroupCache::initialized = false;

GroupCache::~GroupCache()
{
  while (!byName.empty())
  {
    delete byName.begin()->second;
    byName.erase(byName.begin());
  }
}

void GroupCache::Initialize()
{
  logs::debug << "Initialising group cache.." << logs::endl;
  assert(!instance.initialized);
  instance.Replicate();
  Replicator::Register(&instance);
  instance.initialized = true;
}

bool GroupCache::Replicate()
{
  auto now = boost::posix_time::microsec_clock::local_time();
  bool complete = true;

  boost::lock_guard<boost::mutex> createLock(createMutex);
  boost::ptr_vector<acl::Group> groups = db::group::GetAllPtr(lastReplicate);
  
  if (!groups.empty())
  {
    boost::unique_lock<boost::mutex> lock(mutex);
    while (!groups.empty())
    {
      auto group = groups.release(groups.begin());

      auto it = byName.find(group->Name());
      if (it != byName.end())
      {
        if (group->Modified() >= it->second->Modified())
        {
          delete it->second;
          it->second = group.get();
          byGID.at(group->GID()) = group.get();
        }
        else
        {
          complete = false;
          continue;
        }
      }
      else
      {
        byGID.insert(std::make_pair(group->GID(), group.get()));
        byName.insert(std::make_pair(group->Name(), group.get()));
      }
      
      group.release();
    }  
  }
  
  lastReplicate = now;
  return complete;
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
  boost::lock_guard<boost::mutex> createLock(instance.createMutex);
  try
  {
    std::unique_ptr<acl::Group> group(new acl::Group(name));
    if (Exists(group->Name()) ||  !db::group::Create(*group))
      return util::Error::Failure("Group " + name + " already exists.");
    
    {
      boost::lock_guard<boost::mutex> lock(instance.mutex);
      assert(instance.byName.find(name) == instance.byName.end());

      instance.byName.insert(std::make_pair(group->Name(), group.get()));
      instance.byGID.insert(std::make_pair(group->GID(), group.get()));
      
      Save(*group);
      db::groupprofile::Save(GroupProfile(group->GID()));
      group.release();
    }
  }
  catch (const util::RuntimeError& e)
  {
    return util::Error::Failure(e.Message());
  }
  
  return util::Error::Success();
}

util::Error GroupCache::Delete(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end())
    return util::Error::Failure("Group " + name + " doesn't exist.");
  
  if (it->second->GID() == 0) return util::Error::Failure("Cannot delete root group with GID 0.");
    
  acl::GroupID gid = it->second->GID();
  db::group::Delete(gid);
  db::groupprofile::Delete(gid);
  
  instance.byGID.erase(instance.byGID.find(gid));
  delete it->second;
  instance.byName.erase(it);
    
  return util::Error::Success();
}

util::Error GroupCache::Rename(const std::string& oldName, const std::string& newName)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  if (instance.byName.find(newName) != instance.byName.end())
    return util::Error::Failure("New name " + newName + " taken by another group.");
  
  ByNameMap::iterator it = instance.byName.find(oldName);
  if (it == instance.byName.end()) return util::Error::Failure("Group " + oldName + " doesn't exist.");

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

  if (it == instance.byName.end()) 
    throw util::RuntimeError("Group (" + name + ") doesn't exist.");

  return *it->second;
}

acl::Group GroupCache::Group(GroupID gid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByGIDMap::iterator it = instance.byGID.find(gid);
  if (it == instance.byGID.end())
  {
    std::ostringstream os;
    os << "Group with gid " << gid << " doesn't exist.";
    throw util::RuntimeError(os.str());
  }
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
  if (gid == -1) return "NoGroup";
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByGIDMap::iterator it = instance.byGID.find(gid);
  if (it == instance.byGID.end()) return "UNKNOWN";
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
