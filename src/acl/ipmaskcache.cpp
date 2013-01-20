#include <ostream>
#include "acl/ipmaskcache.hpp"
#include "db/user/user.hpp"
#include "db/user/ipmask.hpp"
#include "util/string.hpp"
#include "logs/logs.hpp"

namespace acl
{
IpMaskCache IpMaskCache::instance;

void IpMaskCache::Initialize()
{
  logs::debug << "Initialising ip mask cache.." << logs::endl;
  boost::unique_lock<boost::shared_mutex> lock(instance.mtx);
  db::ipmask::GetAll(instance.userIPMaskMap);
}

bool IpMaskCache::Check(const std::string& addr)
{
  boost::shared_lock<boost::shared_mutex> lock(instance.mtx);
  for (auto uid: instance.userIPMaskMap)
    for (auto& mask: uid.second)
    {
      if (util::string::WildcardMatch(mask, addr, false))
        return true; 
    }
  return false;
}


util::Error IpMaskCache::Add(const acl::User& user, const std::string& mask,
  std::vector<std::string>& deleted)
{
  deleted.clear();
  boost::upgrade_lock<boost::shared_mutex> lock(instance.mtx);
  UserIPMaskMap::iterator masks = instance.userIPMaskMap.find(user.UID());
  if (masks == instance.userIPMaskMap.end())
  {
    boost::upgrade_to_unique_lock<boost::shared_mutex> writeLock(lock);
    instance.userIPMaskMap.insert({user.UID(), std::vector<std::string>({mask})});
    return util::Error::Success();
  }

  std::vector<std::string>::iterator it;
  for (it = masks->second.begin();
    it != masks->second.end();)
  {
    // check if there is a broader mask
    if (util::string::WildcardMatch(*it, mask, false))
      return util::Error::Failure("Broader IP mask exists.");
    // check if mask we are adding is broader
    else if (util::string::WildcardMatch(mask, *it, false))
    {
      deleted.push_back(*it);
      db::ipmask::Delete(user, *it);
      {
        boost::upgrade_to_unique_lock<boost::shared_mutex> writeLock(lock);
        it = masks->second.erase(it);
      }
      continue;
    }
    ++it;
  }   

  // check only a maximum of 10 masks per user
  if (instance.userIPMaskMap[user.UID()].size() > 9)
    return util::Error::Failure("10 IP masks already added.");

  {
    boost::upgrade_to_unique_lock<boost::shared_mutex> writeLock(lock);
    instance.userIPMaskMap[user.UID()].push_back(mask);
  }
  db::ipmask::Add(user, mask);
  return util::Error::Success();
}

util::Error IpMaskCache::Delete(const acl::User& user, int idx, std::string& deleted)
{
  boost::upgrade_lock<boost::shared_mutex> lock(instance.mtx);
  UserIPMaskMap::iterator masks = instance.userIPMaskMap.find(user.UID());

  if (masks == instance.userIPMaskMap.end() ||
      static_cast<unsigned int>(idx) >= masks->second.size())
  {
    std::ostringstream output;
    output << "Index out of range, select an index between 0 and ";
    output << masks->second.size() - 1 << ".";
    return util::Error::Failure(output.str());
  }

  deleted = masks->second[idx];
  db::ipmask::Delete(user, masks->second[idx]);
  masks->second.erase(masks->second.begin() + idx);

  return util::Error::Success();
}

util::Error IpMaskCache::DeleteAll(const acl::User& user, std::vector<std::string>& deleted)
{
  boost::upgrade_lock<boost::shared_mutex> lock(instance.mtx);
  UserIPMaskMap::iterator masks = instance.userIPMaskMap.find(user.UID());
  if (masks == instance.userIPMaskMap.end() || masks->second.empty())
    return util::Error::Failure("User " + user.Name() + " has no IP masks.");

  while (!masks->second.empty())
  {
    deleted.push_back(masks->second.front());
    db::ipmask::Delete(user, masks->second.front());
    masks->second.erase(masks->second.begin());
  }

  return util::Error::Success();  
}

util::Error IpMaskCache::List(const acl::User& user,
  std::vector<std::string>& masks)
{
  masks.clear();
  boost::upgrade_lock<boost::shared_mutex> lock(instance.mtx);
  UserIPMaskMap::iterator it = instance.userIPMaskMap.find(user.UID());
  if (it == instance.userIPMaskMap.end())
    return util::Error::Failure("User "+ user.Name() + " has no IP masks.");            
  masks = it->second;
  return util::Error::Success();
}

// end
}

#ifdef ACL_IPMASKQUEUE_TEST

#include "db/interface.hpp"
#include "db/pool.hpp"
#include "acl/usercache.hpp"
#include "logs/logs.hpp"

int main()
{
  db::Initialize();
  acl::IpMaskCache::Initialize();

  logs::debug << acl::IpMaskCache::Check("blah@192.168.1.1") << logs::endl;
  logs::debug << acl::IpMaskCache::Check("*@hello.com") << logs::endl;

  acl::User user = acl::UserCache::User("iotest");

  std::vector<std::string> deleted;
  acl::IpMaskCache::Add(user, "*@*", deleted);
  util::Error err = acl::IpMaskCache::Add(user, "*@192.168.1.*", deleted);
  if (!err) logs::error << err.Message() << logs::endl;

  for (auto i: deleted)
    logs::debug << "Deleted: " << i << logs::endl;
  logs::debug << "Added: *@*" << logs::endl;

  acl::IpMaskCache::Delete(user, "*@*");

     
  acl::IpMaskCache::Add(user, "*@192.168.1.*", deleted);
  acl::IpMaskCache::Add(user, "*@5.168.1.*", deleted);
 
  acl::IpMaskCache::List(user, deleted);
  for (auto i: deleted)
    logs::debug << i << logs::endl;

   

  db::Pool::StopThread();
  return 0;
}

#endif
    
