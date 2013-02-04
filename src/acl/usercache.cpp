#include <sstream>
#include <cassert>
#include <memory>
#include <vector>
#include "acl/usercache.hpp"
#include "acl/groupcache.hpp"
#include "db/user/userprofile.hpp"
#include "db/user/user.hpp"
#include "logs/logs.hpp"
#include "cfg/get.hpp"
#include "ftp/task/task.hpp"
#include "acl/replicator.hpp"
#include "util/string.hpp"
#include "acl/userprofile.hpp"

namespace acl
{

UserCache UserCache::instance;
bool UserCache::initialized = false;

util::Error UserAllowed(const std::string& name)
{
  const cfg::Config& config = cfg::Get();
  if (std::find(config.BannedUsers().begin(), 
      config.BannedUsers().end(), name) !=
      config.BannedUsers().end())
    return util::Error::Failure("User " + name + " is in the banned list.");
  else
    return util::Error::Success();
}

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
  assert(acl::GroupCache::Initialized());

  instance.Replicate();
  Replicator::Register(&instance);
  instance.initialized = true;  
}

bool UserCache::Replicate()
{
  auto now = boost::posix_time::microsec_clock::local_time();
  bool complete = true;
  
  boost::lock_guard<boost::mutex> createLock(createMutex);
  boost::ptr_vector<acl::User> users = db::user::GetAllPtr(lastReplicate);

  int done = 0;
  if (!users.empty())
  {
    boost::unique_lock<boost::mutex> lock(mutex);
    boost::unique_lock<boost::shared_mutex> ipLock(ipMutex);
    while (!users.empty())
    {
      auto user = users.release(users.begin());
      auto it = byName.find(user->Name());
      if (it != byName.end())
      {
        if (user->Modified() >= it->second->Modified())
        {
          delete it->second;
          it->second = user.get();
          byUID.at(user->UID()) = user.get();
          ipMasks.at(user->UID()) = user->ListIPMasks();
        }
        else
        {
          complete = false;
          continue;
        }
      }
      else
      {
        byUID.insert(std::make_pair(user->UID(), user.get()));
        byName.insert(std::make_pair(user->Name(), user.get()));
        ipMasks.insert(std::make_pair(user->UID(), user->ListIPMasks()));
      }
      
      std::make_shared<ftp::task::UserUpdate>(user->UID())->Push();
      user.release();
    }
    ++done;
  }

  lastReplicate = now;
  return complete;
}

void UserCache::Save(const acl::User& user)
{
  db::user::Save(user);
  std::make_shared<ftp::task::UserUpdate>(user.UID())->Push();
}

void UserCache::Save(const acl::User& user, const std::string& field)
{
  db::user::Save(user, field);
  std::make_shared<ftp::task::UserUpdate>(user.UID())->Push();
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
  unsigned totalUsers = cfg::Get().TotalUsers();
  if (totalUsers > 0 && Count() >= totalUsers)
  {
    std::ostringstream os;
    os << "Limit of " << totalUsers << " user(s) has been reached.";
    return util::Error::Failure(os.str());
  }
  
  util::Error e(UserAllowed(name));
  if (!e) return e;
  
  boost::lock_guard<boost::mutex> createLock(instance.createMutex);
  try
  {
    std::unique_ptr<acl::User> user(new acl::User(name, password, flags));
    if (Exists(user->Name()) || !db::user::Create(*user))
        return util::Error::Failure("User " + name + " already exists.");

    {
      boost::lock_guard<boost::mutex> lock(instance.mutex);
      assert(instance.byName.find(name) == instance.byName.end());
        
      instance.byName.insert(std::make_pair(user->Name(), user.get()));
      instance.byUID.insert(std::make_pair(user->UID(), user.get()));
      
      Save(*user);
      UserProfile profile(user->UID(), creator);
      db::userprofile::Save(profile);
      
      boost::lock_guard<boost::shared_mutex> ipLock(instance.ipMutex);
      instance.ipMasks.insert(std::make_pair(user->UID(), std::vector<std::string>()));
      user.release();
    }
  }
  catch (const util::RuntimeError& e)
  {
    return util::Error::Failure(e.Message());
  }
  
  return util::Error::Success();
}

util::Error UserCache::Purge(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  if (!it->second->Deleted()) return util::Error::Failure("User " + name + " must be deleted first.");
  if (it->second->UID() == 0) return util::Error::Failure("Cannot purge root user with UID 0.");
  
  instance.byUID.erase(instance.byUID.find(it->second->UID()));

  acl::UserID uid = it->second->UID();

  delete it->second;
  instance.byName.erase(it);
  
  db::user::Delete(uid);
  db::userprofile::Delete(uid);

  boost::unique_lock<boost::shared_mutex> ipLock(instance.ipMutex);
  instance.ipMasks.erase(uid);
    
  return util::Error::Success();
}

util::Error UserCache::Delete(const std::string& name)
{
  if (cfg::Get().IsMaster(name)) return util::Error::Failure("Cannot delete a master.");
  
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) 
    return util::Error::Failure("User " + name + " doesn't exist.");

  if (it->second->Deleted()) 
    return util::Error::Failure("User " + name + " already deleted.");
  it->second->AddFlag(Flag::Deleted);

  Save(*it->second, "flags");

  return util::Error::Success();
}

util::Error UserCache::Readd(const std::string& name)
{
  unsigned totalUsers = cfg::Get().TotalUsers();
  if (totalUsers > 0 && Count() >= totalUsers)
  {
    std::ostringstream os;
    os << "Limit of " << totalUsers << " user(s) has been reached.";
    return util::Error::Failure(os.str());
  }

  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end())
    return util::Error::Failure("User " + name + " doesn't exist.");
  
  if (!it->second->Deleted()) 
    return util::Error::Failure("User " + name + " is not deleted.");
  it->second->DelFlag(Flag::Deleted);
  
  Save(*it->second, "flags");

  return util::Error::Success();
}

util::Error UserCache::Rename(const std::string& oldName, const std::string& newName)
{
  util::Error e(UserAllowed(newName));
  if (!e) return e;

  boost::lock_guard<boost::mutex> lock(instance.mutex);
  if (instance.byName.find(newName) != instance.byName.end())
    return util::Error::Failure("New name " + newName + " taken by another user.");
  
  ByNameMap::iterator it = instance.byName.find(oldName);
  if (it == instance.byName.end()) return util::Error::Failure("User " + oldName + " doesn't exist.");

  it->second->SetName(newName);
  instance.byName.insert(std::make_pair(newName, it->second));
  instance.byName.erase(it);
  
  Save(*it->second, "name");

  return util::Error::Success();
}

util::Error UserCache::SetPassword(const std::string& name, const std::string& password)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  
  it->second->SetPassword(password);
  
  Save(*it->second, "password");
  Save(*it->second, "salt");

  return util::Error::Success();
}

util::Error UserCache::SetFlags(const std::string& name, const std::string& flags)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  
  it->second->SetFlags(flags);

  Save(*it->second, "flags");
  
  return util::Error::Success();
}

util::Error UserCache::AddFlags(const std::string& name, const std::string& flags)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  
  it->second->AddFlags(flags);
  
  Save(*it->second, "flags");

  return util::Error::Success();
}

util::Error UserCache::DelFlags(const std::string& name, const std::string& flags)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  
  it->second->DelFlags(flags);
  
  Save(*it->second, "flags");

  return util::Error::Success();
}

util::Error UserCache::SetPrimaryGID(const std::string& name, GroupID gid, GroupID oldPrimaryGID)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  
  oldPrimaryGID = it->second->PrimaryGID();
  if (oldPrimaryGID != -1)
  {
    it->second->AddSecondaryGID(oldPrimaryGID);
    Save(*it->second, "secondary gids");
  }
  
  it->second->SetPrimaryGID(gid);
  Save(*it->second, "primary gid");

  return util::Error::Success();
}

util::Error UserCache::AddGID(const std::string& name, GroupID gid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  
  acl::User& user = *it->second;

  if (user.HasGID(gid))
  {
    return util::Error::Failure("Already a member.");
  }

  if (user.PrimaryGID() == -1)
  {
    user.SetPrimaryGID(gid);
    Save(*it->second, "primary gid");
  }
  else
  {
    user.AddSecondaryGID(gid);
    Save(*it->second, "secondary gids");
  }

  return util::Error::Success();
}

util::Error UserCache::DelGID(const std::string& name, GroupID gid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");

  acl::User& user = *it->second;
  if (!user.HasGID(gid)) return util::Error::Failure("Not a member.");
  
  if (user.PrimaryGID() == gid)
  {
    if (!user.SecondaryGIDs().empty())
    {
      user.SetPrimaryGID(user.SecondaryGIDs().front());
      user.DelSecondaryGID(user.PrimaryGID());
      Save(*it->second, "secondary gids");
    }
    else
      user.SetPrimaryGID(-1);
    Save(*it->second, "primary gid");
  }
  else
  {
    user.DelSecondaryGID(gid);
    Save(*it->second, "secondary gids");
  }
  

  return util::Error::Success();
}

util::Error UserCache::ResetGIDs(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");

  it->second->SetPrimaryGID(-1);
  it->second->ResetSecondaryGIDs();

  Save(*it->second, "secondary gids");

  return util::Error::Success();
}

util::Error UserCache::ToggleGadminGID(const std::string& name, GroupID gid, bool& added)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");
  
  acl::User& user = *it->second;

  if (!user.HasGID(gid)) return util::Error::Failure("Not a member of that group.");  

  added = !user.HasGadminGID(gid);
  if (added) user.AddGadminGID(gid);
  else user.DelGadminGID(gid); 

  Save(*it->second, "flags  ");
  Save(*it->second, "gadmin gids");
  return util::Error::Success();
}

/*util::Error UserCache::DelGadminGID(const std::string& name, GroupID gid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return util::Error::Failure("User " + name + " doesn't exist.");

  acl::User& user = *it->second;
  if (!user.HasGadminGID(gid)) return util::Error::Failure("Not gadmin.");
  
  user.DelGadminGID(gid);
  Save(*it->second, "gadmin gids");
  return util::Error::Success();
}*/

acl::User UserCache::User(const std::string& name)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end())
    throw util::RuntimeError("User " + name + " doesn't exist");
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

GroupID UserCache::PrimaryGID(UserID uid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByUIDMap::iterator it = instance.byUID.find(uid);
  if (it == instance.byUID.end()) return -1;
  return it->second->PrimaryGID();
}

bool UserCache::HasGID(const std::string& name, acl::GroupID gid)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  ByNameMap::iterator it = instance.byName.find(name);
  if (it == instance.byName.end()) return false;
  return it->second->HasGID(gid);
}

unsigned UserCache::Count(bool includeDeleted)
{
  boost::lock_guard<boost::mutex> lock(instance.mutex);
  if (includeDeleted) return instance.byUID.size();
  unsigned count = 0;
  for (auto& kv : instance.byUID)
  {
    if (!kv.second->Deleted()) ++count;
  }
  return count;
}

bool UserCache::IPAllowed(const std::string& address)
{
  std::string identAddress = "*@" + address;
  boost::shared_lock<boost::shared_mutex> lock(instance.ipMutex);
  for (auto uid : instance.ipMasks)
  {
    for (auto& mask : uid.second)
    {
      if (util::string::WildcardMatch(mask, identAddress))
        return true; 
    }
  }
  return false;
}

bool UserCache::IdentIPAllowed(acl::UserID uid, const std::string& identAddress)
{
  boost::shared_lock<boost::shared_mutex> lock(instance.ipMutex);
  auto it = instance.ipMasks.find(uid);
  if (it != instance.ipMasks.end())
  {
    for (const std::string& mask : it->second)
    {
      if (util::string::WildcardMatch(mask, identAddress))
        return true;     
    }
  }
  return false;
}

util::Error UserCache::AddIPMask(const std::string& name, const std::string& mask,
                                 std::vector<std::string>& redundant)
{
  redundant.clear();
  
  {
    boost::lock_guard<boost::mutex> lock(instance.mutex);
    auto it = instance.byName.find(name);
    if (it == instance.byName.end()) 
      return util::Error::Failure("User " + name + " doesn't exist.");
    
    acl::User& user = *it->second;
    
    auto e = user.AddIPMask(mask, redundant);
    if (!e) return e;  

    {
      boost::unique_lock<boost::shared_mutex> ipLock(instance.ipMutex);
      auto it = instance.ipMasks.find(user.UID());
      if (it == instance.ipMasks.end())
      {
        assert(redundant.empty());
        instance.ipMasks.insert(std::make_pair(user.UID(), std::vector<std::string>{mask}));
      }
      else
      {
        auto& masks = it->second;
        masks.erase(std::remove_if(masks.begin(), masks.end(),
              [redundant](const std::string& mask)
              {
                return std::find(redundant.begin(), redundant.end(), mask) != redundant.end();
              }), masks.end());
        masks.emplace_back(mask);
      }
    }
    
    db::user::SaveIPMasks(user);
  }
  
  return util::Error::Success();
}

util::Error UserCache::AddIPMask(const std::string& name, const std::string& mask)
{
  std::vector<std::string> redundant;
  return AddIPMask(name, mask, redundant);
}

util::Error UserCache::DelIPMask(const std::string& name, int index, std::string& deleted)
{
  {
    boost::lock_guard<boost::mutex> lock(instance.mutex);
    auto it = instance.byName.find(name);
    if (it == instance.byName.end()) 
      return util::Error::Failure("User " + name + " doesn't exist.");
    
    acl::User& user = *it->second;
    
    auto e = user.DelIPMask(index, deleted);
    if (!e) return e;  

    {
      boost::unique_lock<boost::shared_mutex> ipLock(instance.ipMutex);
      auto it = instance.ipMasks.find(user.UID());
      assert(it != instance.ipMasks.end());
      
      auto& masks = it->second;
      masks.erase(std::remove(masks.begin(), masks.end(), deleted), masks.end());
    }
    
    db::user::SaveIPMasks(user);
  }
  
  return util::Error::Success();
}

util::Error UserCache::DelAllIPMasks(const std::string& name, std::vector<std::string>& deleted)
{
  {
    boost::lock_guard<boost::mutex> lock(instance.mutex);
    auto it = instance.byName.find(name);
    if (it == instance.byName.end()) 
      return util::Error::Failure("User " + name + " doesn't exist.");
    
    acl::User& user = *it->second;
    
    user.DelAllIPMasks(deleted);

    {
      boost::unique_lock<boost::shared_mutex> ipLock(instance.ipMutex);
      auto it = instance.ipMasks.find(user.UID());
      if (it != instance.ipMasks.end()) it->second.clear();
    }
  }
  
  return util::Error::Success();  
}

util::Error UserCache::ListIPMasks(const std::string& name, std::vector<std::string>& masks)
{
  {
    boost::lock_guard<boost::mutex> lock(instance.mutex);
    auto it = instance.byName.find(name);
    if (it == instance.byName.end()) 
      return util::Error::Failure("User " + name + " doesn't exist.");
      
    masks = it->second->ListIPMasks();
  }
  
  return util::Error::Success();
}

} /* acl namespace */
