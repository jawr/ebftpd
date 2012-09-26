#ifndef __FS_OWNER_HPP
#define __FS_OWNER_HPP

#include <string>
#include <ostream>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp>
#include "boost/serialization/unordered_map.hpp"
#include "acl/types.hpp"
#include "fs/path.hpp"
#include "fs/filelock.hpp"
#include "util/mrucache.hpp"

namespace fs
{

class Owner
{
  uid_t uid;
  gid_t gid;

  template <class Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & uid;
    ar & gid;
    
    (void) version;
  }

  Owner() : uid(0), gid(0) { }
  
public:
  Owner(uid_t uid, gid_t gid) : uid(uid), gid(gid) { }
  
  uid_t UID() const { return uid; }
  gid_t GID() const { return gid; }

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
  
  boost::unordered_map<std::string, OwnerEntry> entries;
  
  void Create(const std::string& name, const Owner& owner);

  template <class Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & entries;
    
    (void) version;
  }
  
  static const std::string ownerFilename;
  
  bool InnerLoad(FileLockPtr& lock);
  bool InnerSave(FileLockPtr& lock);
  
public:
  OwnerFile(const Path& parent) :
    parent(parent), ownerFile(parent / ownerFilename) { }
  
  void Chown(const std::string& name, const Owner& owner);
  void Delete(const std::string& name);
  bool Exists(const std::string& name) const;
  const fs::Owner& Owner(const std::string& name) const;

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
  util::MRUCache<std::string, CacheEntry> cache;
  
  static const uint16_t cacheSize = 1000;
  
  static OwnerCache instance;
  
  OwnerCache() : needSave(false), cache(cacheSize) { }

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
