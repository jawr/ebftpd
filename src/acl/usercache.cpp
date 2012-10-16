#include <memory>
#include <vector>
#include "acl/usercache.hpp"
#include "acl/groupcache.hpp"
#include "db/interface.hpp"
#include "logs/logs.hpp"

namespace acl
{

UserCache UserCache::instance;
bool UserCache::initalized = false;

UserCache::~UserCache()
{
  while (!byName.empty())
  {
    delete byName.begin()->second;
    byName.erase(byName.begin());
  }
}

void UserCache::Initalize()
{
  // grab all user's from the database and populate the map
  if (instance.initalized) return;
  boost::lock_guard<boost::mutex> lock(instance.mutex);

  // need to initalize group cache first
  acl::GroupCache::Initalize();

  
  boost::ptr_vector<acl::User> users;
  db::GetUsers(users);
  while (!users.empty())
  {
    auto user = users.release(users.begin());
    acl::GroupCache::AddUIDToGroup(user->PrimaryGID(), user->UID());
    for (auto gid: user->SecondaryGIDs())
      acl::GroupCache::AddUIDToGroup(gid, user->UID());

    instance.byUID.insert(std::make_pair(user->UID(), user.get()));
    instance.byName.insert(std::make_pair(user->Name(), user.get()));
    user.release(); 
  }
  instance.initalized = true;
  
}

void UserCache::Save(const acl::User& user)
{
  db::SaveUser(user);
}

bool UserCache::Exists(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byName.find(name) != instance.byName.end();
}

bool UserCache::Exists(UserID uid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  return instance.byUID.find(uid) != instance.byUID.end();
}

util::Error UserCache::Create(const std::string& name, const std::string& password,
                              const std::string& flags)
{
  acl::UserID uid = db::GetNewUserID();

  boost::lock_guard<boost::mutex> lock(instance.mutex);
  if (instance.byName.find(name) != instance.byName.end())
    return util::Error::Failure("User already exists");

  std::unique_ptr<acl::User> user(new acl::User(name, uid, password, flags));
    
  instance.byName.insert(std::make_pair(name, user.get()));
  instance.byUID.insert(std::make_pair(uid, user.get()));
  
  Save(*user.release());
  
  return util::Error::Success();
}

util::Error UserCache::Purge(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User doesn't exist");
  if (!it->second->Deleted()) return util::Error::Failure("User has not been deleted");

  instance.byUID.erase(instance.byUID.find(it->second->UID()));

  acl::UserID uid = it->second->UID();

  delete it->second;
  instance.byName.erase(it);
  
  db::DeleteUser(uid);
  
  return util::Error::Success();
}

util::Error UserCache::Delete(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User doesn't exist");
  
  if (it->second->Deleted()) return util::Error::Failure("User already deleted");
  it->second->AddFlag(Flag::Deleted);
  
  Save(*it->second);
  
  return util::Error::Success();
}

util::Error UserCache::Readd(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User doesn't exist");
  
  if (!it->second->Deleted()) return util::Error::Failure("User not deleted");
  it->second->DelFlag(Flag::Deleted);
  
  Save(*it->second);
  
  return util::Error::Success();
}

util::Error UserCache::Rename(const std::string& oldName, const std::string& newName)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  if (instance.byName.find(newName) != instance.byName.end())
    return util::Error::Failure("User already exists with new name");
  
  ByNameMap::iterator it = instance.byName.find(oldName);
  if (it == instance.byName.end()) return util::Error::Failure("User doesn't exist");

  it->second->SetName(newName);
  instance.byName.insert(std::make_pair(newName, it->second));
  instance.byName.erase(it);
  
  Save(*it->second);
  
  return util::Error::Success();
}

util::Error UserCache::SetPassword(const std::string& name, const std::string& password)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User doesn't exist");
  
  it->second->SetPassword(password);
  
  Save(*it->second);
  
  return util::Error::Success();
}

util::Error UserCache::SetFlags(const std::string& name, const std::string& flags)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User doesn't exist");
  
  it->second->SetFlags(flags);
  
  Save(*it->second);
  
  return util::Error::Success();
}

util::Error UserCache::AddFlags(const std::string& name, const std::string& flags)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User doesn't exist");
  
  it->second->AddFlags(flags);
  
  Save(*it->second);
  
  return util::Error::Success();
}

util::Error UserCache::DelFlags(const std::string& name, const std::string& flags)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User doesn't exist");
  
  it->second->DelFlags(flags);
  
  Save(*it->second);
  
  return util::Error::Success();
}

util::Error UserCache::SetPrimaryGID(const std::string& name, GroupID gid)
{
  // unlike to happen, but what to do if group is deleted between 
  // checking it exists and then this function being called?
  // same for secondary gid functions

  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User doesn't exist");
  
  it->second->SetPrimaryGID(gid);
  
  Save(*it->second);
  
  return util::Error::Success();
}

util::Error UserCache::AddSecondaryGID(const std::string& name, GroupID gid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User doesn't exist");
  
  it->second->AddSecondaryGID(gid);
  
  Save(*it->second);
  
  return util::Error::Success();
}

util::Error UserCache::DelSecondaryGID(const std::string& name, GroupID gid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User doesn't exist");
  
  it->second->DelSecondaryGID(gid);
  
  Save(*it->second);
  
  return util::Error::Success();
}

acl::User UserCache::User(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) throw util::RuntimeError("User doesn't exist");
  return *it->second;
}

acl::User UserCache::User(UserID uid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByUIDMap::iterator it = instance.byUID.find(uid);
  if (it == instance.byUID.end()) throw util::RuntimeError("User doesn't exist");
  return *it->second;
}

UserID UserCache::NameToUID(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return -1;
  return it->second->UID();
}

std::string UserCache::UIDToName(UserID uid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByUIDMap::iterator it = instance.byUID.find(uid);
  if (it == instance.byUID.end()) return "unknown";
  return it->second->Name();
}

} /* acl namespace */


#ifdef ACL_USERCACHE_TEST

int main()
{
  using namespace acl;
  
  std::cout << "exists: " << UserCache::Exists("someone") << std::endl;
  std::cout << "exists: " << UserCache::Exists(123) << std::endl;
  
  std::cout << "create: " << UserCache::Create("someone", "somepass", "1") << std::endl;
  std::cout << "exists: " << UserCache::Exists("someone") << std::endl;
  
  User user = UserCache::User("someone");
  std::cout << "exists: " << UserCache::Exists(user.UID()) << std::endl;
  std::cout << "rename: " << UserCache::Rename("someone" ,"someoneelse") << std::endl;
  std::cout << "exists: " << UserCache::Exists("someoneelse") << std::endl;
  std::cout << "exists: " << UserCache::Exists(user.UID()) << std::endl;
  std::cout << "exists: " << UserCache::Exists("someone") << std::endl;
  UserCache::Delete("someoneelse");
  std::cout << "exists: " << UserCache::Exists("someoneelse") << std::endl;
  std::cout << "exists: " << UserCache::Exists(user.UID()) << std::endl;
  
  
}

#endif
