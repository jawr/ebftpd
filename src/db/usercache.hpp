#ifndef __DB_USERCACHE_HPP
#define __DB_USERCACHE_HPP

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

struct UserCacheBase
{
  virtual ~UserCacheBase() { }
  virtual std::string UIDToName(acl::UserID uid) = 0;
  virtual acl::UserID NameToUID(const std::string& name) = 0;
  virtual acl::GroupID UIDToPrimaryGID(acl::UserID uid) = 0;
  virtual bool IdentIPAllowed(const std::string& identAddress) = 0;  
  virtual bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid) = 0;  
};

class UserCache : 
  public UserCacheBase,
  public Replicable
{
  boost::mutex namesMutex;
  std::unordered_map<acl::UserID, std::string> names;

  boost::mutex uidsMutex;
  std::unordered_map<std::string, acl::UserID> uids;
  
  boost::mutex primaryGidsMutex;
  std::unordered_map<acl::UserID, acl::GroupID> primaryGids;

  boost::mutex ipMasksMutex;
  std::unordered_map<acl::UserID, std::vector<std::string>> ipMasks;
  
public:  
  UserCache() : Replicable("users") { }
  std::string UIDToName(acl::UserID uid);
  acl::UserID NameToUID(const std::string& name);
  acl::GroupID UIDToPrimaryGID(acl::UserID uid);  
  bool IdentIPAllowed(const std::string& identAddress);
  bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid);

  bool Replicate(const mongo::BSONElement& id);
  bool Populate();
};

struct UserNoCache : public UserCacheBase
{
  std::string UIDToName(acl::UserID uid);
  acl::UserID NameToUID(const std::string& name);
  acl::GroupID UIDToPrimaryGID(acl::UserID uid);  
  bool IdentIPAllowed(const std::string& identAddress);
  bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid);
};

} /* db namespace */

#endif
