#ifndef __FS_OWNER_HPP
#define __FS_OWNER_HPP

#include <string>
#include <ostream>
#include <unordered_map>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp>
#include "boost/serialization/unordered_map.hpp"
#include "acl/types.hpp"
#include "fs/path.hpp"
#include "fs/filelock.hpp"
#include "util/lrucache.hpp"

namespace fs
{

class Owner
{
  acl::UserID uid;
  acl::GroupID gid;

  template <class Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & uid;
    ar & gid;
    
    (void) version;
  }

  Owner() : uid(0), gid(0) { }
  
public:
  Owner(acl::UserID uid, acl::GroupID gid) : uid(uid), gid(gid) { }
  
  acl::UserID UID() const { return uid; }
  acl::GroupID GID() const { return gid; }

  friend class boost::serialization::access;
};
  
class OwnerEntry
{
  std::string name;
  fs::Owner owner;

  template <class Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & name;
    ar & owner;
    
    (void) version;
  }

public:
  OwnerEntry() : owner(-1, -1) { }

  OwnerEntry(const std::string& name, const fs::Owner& owner) :
    name(name), owner(owner) { }
    
  const std::string& Name() const { return name; }
  const fs::Owner& Owner() const { return owner; }
  
  void Chown(const fs::Owner& owner) { this->owner = owner; }

  friend class boost::serialization::access;
};

struct OwnerFile
{
  std::string parent;
  std::string ownerFile;
  
  std::unordered_map<std::string, OwnerEntry> entries;
  
  void Create(const std::string& name, const Owner& owner);

  template <class Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & entries;
    
    (void) version;
  }
  
  
  bool InnerLoad(FileLockPtr& lock);
  bool InnerSave(FileLockPtr& lock);
  
public:
  static const std::string ownerFilename;

  OwnerFile(const Path& parent) :
    parent(parent), ownerFile(parent / ownerFilename) { }
  
  void Chown(const std::string& name, const Owner& owner);
  void Delete(const std::string& name);
  bool Exists(const std::string& name) const;
  fs::Owner Owner(const std::string& name) const;

  bool Load(FileLockPtr& lock);
  bool Load();
  bool Save(FileLockPtr& lock);
  bool Save();
  
  friend class boost::serialization::access;
};

class OwnerCache
{
  boost::thread thread;
  boost::shared_mutex cacheMutex;
  boost::condition_variable_any saveCond;
  bool needSave;
  
  typedef std::pair<OwnerFile*, bool> CacheEntry;
  util::LRUCache<std::string, CacheEntry> cache;
  
  static const uint16_t cacheSize = 1000;
  
  static OwnerCache instance;
  
  OwnerCache() : needSave(false), cache(cacheSize) { }

  static bool GetParentName(const fs::Path& path, fs::Path& parent, fs::Path& name);
  
  void Main();
  
public:  
  static void Start();
  static void Stop();
  
  static void Chown(const Path& path, const Owner& owner);
  static void Delete(const Path& path);
  static fs::Owner Owner(const Path& path);
};

std::ostream& operator<<(std::ostream& os, const Owner& owner);

} /* fs namespace */



#endif
