#include <sstream>
#include <cassert>
#include <memory>
#include <vector>
#include "acl/usercache.hpp"
#include "acl/groupcache.hpp"
#include "db/user/userprofile.hpp"
#include "db/user/user.hpp"
#include "logs/logs.hpp"

namespace acl
{

UserCache UserCache::instance;
bool UserCache::initialized = false;

UserCache::~UserCache()
{
  while (!byName.empty())
  {
    delete byName.begin()->second;
    byName.erase(byName.begin());
  }
}

void UserCache::Initialize()
{
  logs::debug << "Initialising user cache.." << logs::endl;
  // grab all user's from the database and populate the map
  assert(!instance.initialized);
  boost::lock_guard<boost::mutex> lock(instance.mutex);

  assert(acl::GroupCache::Initialized());

  boost::ptr_vector<acl::User> users;
  db::user::GetAll(users);
  while (!users.empty())
  {
    auto user = users.release(users.begin());

    instance.byUID.insert(std::make_pair(user->UID(), user.get()));
    instance.byName.insert(std::make_pair(user->Name(), user.get()));
    user.release(); 
  }
  
  instance.initialized = true;  
}

void UserCache::Save(const acl::User& user)
{
  db::user::Save(user);
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
                              const std::string& flags, acl::UserID creator)
{
  acl::UserID uid = db::user::GetNewUserID();

  {
    boost::lock_guard<boost::mutex> lock(instance.mutex);
    if (instance.byName.find(name) != instance.byName.end())
      return util::Error::Failure("User " + name + " already exists.");

    std::unique_ptr<acl::User> user(new acl::User(name, uid, password, flags));
      
    instance.byName.insert(std::make_pair(name, user.get()));
    instance.byUID.insert(std::make_pair(uid, user.get()));
    
    Save(*user.release());
    UserProfile profile(uid, creator);
    db::userprofile::Save(profile);
    return util::Error::Success();
  }
}

util::Error UserCache::Purge(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + "doesn't exist.");
  if (!it->second->Deleted()) return util::Error::Failure("User " + name + " must be deleted first.");

  instance.byUID.erase(instance.byUID.find(it->second->UID()));

  acl::UserID uid = it->second->UID();

  delete it->second;
  instance.byName.erase(it);
  
  db::user::Delete(uid);
  
  return util::Error::Success();
}

util::Error UserCache::Delete(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) 
    return util::Error::Failure("User " + name + "doesn't exist.");
  
  if (it->second->Deleted()) 
    return util::Error::Failure("User " + name + " already deleted.");
  it->second->AddFlag(Flag::Deleted);

  db::user::Save(*it->second, "flags");

  return util::Error::Success();
}

util::Error UserCache::Readd(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end())
    return util::Error::Failure("User " + name + "doesn't exist.");
  
  if (!it->second->Deleted()) 
    return util::Error::Failure("User " + name + "is not deleted.");
  it->second->DelFlag(Flag::Deleted);
  
  db::user::Save(*it->second, "flags");

  return util::Error::Success();
}

util::Error UserCache::Rename(const std::string& oldName, const std::string& newName)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  if (instance.byName.find(newName) != instance.byName.end())
    return util::Error::Failure("New name " + newName + "taken by another user.");
  
  ByNameMap::iterator it = instance.byName.find(oldName);
  if (it == instance.byName.end()) return util::Error::Failure("User " + oldName + " doesn't exist.");

  it->second->SetName(newName);
  instance.byName.insert(std::make_pair(newName, it->second));
  instance.byName.erase(it);
  
  db::user::Save(*it->second, "name");

  return util::Error::Success();
}

util::Error UserCache::SetPassword(const std::string& name, const std::string& password)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  
  it->second->SetPassword(password);
  
  db::user::Save(*it->second, "password");
  db::user::Save(*it->second, "salt");

  return util::Error::Success();
}

util::Error UserCache::SetFlags(const std::string& name, const std::string& flags)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  
  it->second->SetFlags(flags);

  db::user::Save(*it->second, "flags");
  
  return util::Error::Success();
}

util::Error UserCache::AddFlags(const std::string& name, const std::string& flags)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  
  it->second->AddFlags(flags);
  
  db::user::Save(*it->second, "flags");

  return util::Error::Success();
}

util::Error UserCache::DelFlags(const std::string& name, const std::string& flags)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  
  it->second->DelFlags(flags);
  
  db::user::Save(*it->second, "flags");

  return util::Error::Success();
}

util::Error UserCache::SetPrimaryGID(const std::string& name, GroupID gid, GroupID& oldPrimaryGID)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  
  oldPrimaryGID = it->second->PrimaryGID();
  if (oldPrimaryGID != -1)
  {
    it->second->AddSecondaryGID(oldPrimaryGID);
    db::user::Save(*it->second, "secondary gids");
  }
  
  it->second->SetPrimaryGID(gid);
  db::user::Save(*it->second, "primary gid");

  return util::Error::Success();
}

util::Error UserCache::AddSecondaryGID(const std::string& name, GroupID gid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  
  it->second->AddSecondaryGID(gid);
  
  db::user::Save(*it->second, "secondary gids");

  return util::Error::Success();
}

util::Error UserCache::DelSecondaryGID(const std::string& name, GroupID gid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  
  it->second->DelSecondaryGID(gid);
  
  db::user::Save(*it->second, "secondary gids");

  return util::Error::Success();
}

util::Error UserCache::ResetSecondaryGIDs(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");

  it->second->ResetSecondaryGIDs();

  db::user::Save(*it->second, "secondary gids");

  return util::Error::Success();
}

util::Error UserCache::IncrCredits(const std::string& name, long long kbytes)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end())
    return util::Error::Failure("Unable to update credits, user " + 
                                name + " doesn't exist.");
  it->second->IncrCredits(kbytes);
  Save(*it->second);
  return util::Error::Success();
}

util::Error UserCache::DecrCredits(const std::string& name, long long kbytes)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) 
    return util::Error::Failure("Unable to update credits, user " + 
                                name + " doesn't exist.");
  it->second->DecrCredits(kbytes);
  Save(*it->second);
  return util::Error::Success();
}

acl::User UserCache::User(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) throw util::RuntimeError("User " + name + " doesn't exist.");
  return *it->second;
}

acl::User UserCache::User(UserID uid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByUIDMap::iterator it = instance.byUID.find(uid);
  if (it == instance.byUID.end())
  {
    std::stringstream os;
    os << "User with uid " << uid << " doesn't exist.";
    throw util::RuntimeError(os.str());
  }
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
