#include <fstream>
#include <functional>
#if defined(TEXT_OWNER_FILES)
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#else
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif
#include "fs/owner.hpp"
#include "util/error.hpp"
#include "logs/logs.hpp"
#include "fs/status.hpp"

#include "acl/usercache.hpp"

namespace fs
{

OwnerCache OwnerCache::instance;
const std::string OwnerFile::ownerFilename = ".owner";

void OwnerFile::Create(const Path& name, const class Owner& owner)
{
  entries.insert(std::make_pair(name.ToString(), OwnerEntry(name.ToString(), owner)));
}

void OwnerFile::Chown(const Path& name, const class Owner& owner)
{
  if (!Exists(name)) Create(name, owner);
  else entries.at(name.ToString()).Chown(owner);
}

void OwnerFile::Delete(const Path& name)
{
  entries.erase(name.ToString());
}

bool OwnerFile::Exists(const Path& name) const
{
  return entries.find(name.ToString()) != entries.end();
}

class fs::Owner OwnerFile::Owner(const Path& name) const
{
  if (Exists(name)) return entries.at(name.ToString()).Owner();
  else return fs::Owner(0, 0);
}

bool OwnerFile::InnerLoad(FileLockPtr& lock)
{
  try
  {
    Status status(ownerFile);
    if (!status.IsRegularFile()) return false;
  }
  catch (const util::SystemError& e)
  {
    return e.Errno() == ENOENT;
  }
  
  std::ifstream fin(ownerFile.ToString().c_str());
  if (!fin) return false;
  
  try
  {
    lock = FileLock::Create(ownerFile.ToString());
  }
  catch (const util::SystemError&)
  {
    return false;
  }

  try
  {
#if defined(TEXT_OWNER_FILES)
    boost::archive::text_iarchive ia(fin);
#else
    boost::archive::binary_iarchive ia(fin);
#endif
    ia >> *this;
  }
  catch (const boost::archive::archive_exception&)
  {
    return false;
  }
  
  return !fin.bad() && !fin.fail();
}

bool OwnerFile::Load(FileLockPtr& lock)
{
  if (!InnerLoad(lock))
  {
    logs::error << "Unable to load owner file: " << ownerFile << logs::endl;
    return false;
  }
  return true;
}

bool OwnerFile::Load()
{
  FileLockPtr lock;
  return Load(lock);
}

bool OwnerFile::InnerSave(FileLockPtr& lock)
{
  std::ofstream fout(ownerFile.ToString().c_str());
  if (!fout) return false;
  
  if (!lock.get())
  {
    try
    {
      lock = FileLock::Create(ownerFile.ToString());
    }
    catch(const util::SystemError&)
    {
      return false;
    }
  }
  try
  {
#if defined(TEXT_OWNER_FILES)
    boost::archive::text_oarchive oa(fout);
#else
    boost::archive::binary_oarchive oa(fout);
#endif
    oa << *this;
  }
  catch (const boost::archive::archive_exception&)
  {
    return false;
  }
  
  return !fout.bad() && !fout.fail();
}

bool OwnerFile::Save(FileLockPtr& lock)
{
  if (!InnerSave(lock))
  {
    logs::error << "Unable to save owner file: " << ownerFile << logs::endl;
    return false;
  }
  return true;
}

bool OwnerFile::Save()
{
  FileLockPtr lock;
  return Save(lock);
}

void OwnerCache::Chown(const RealPath& path, const fs::Owner& owner)
{
  RealPath parent;
  Path name;
  if (!GetParentName(path, parent, name)) return;

  boost::upgrade_lock<boost::shared_mutex> readLock(instance.cacheMutex);
  try
  {
    CacheEntry& entry = instance.cache.Lookup(parent.ToString());
    
    boost::upgrade_to_unique_lock<boost::shared_mutex> writeLock(readLock);
    entry.first->Chown(name, owner);
    entry.second = true; // save please!
  }
  catch (const std::out_of_range&)
  {
    std::unique_ptr<OwnerFile> ownerFile(new OwnerFile(parent));
    if (!ownerFile->Load()) return;
    ownerFile->Chown(name, owner);
    CacheEntry entry = std::make_pair(ownerFile.release(), true /* save please */);
    
    boost::upgrade_to_unique_lock<boost::shared_mutex> writeLock(readLock);
    instance.cache.Insert(parent.ToString(), entry);
  }
  
  instance.needSave = true;
  instance.saveCond.notify_one();
}

bool OwnerCache::GetParentName(const RealPath& path, RealPath& parent, Path& name)
{
  fs::Status status;
  try
  {
    status.Reset(path);
  }
  catch (const util::SystemError&)
  {
    return false;
  }

  if (status.IsDirectory())
  {
    parent = path;
    name = Path(".");
  }
  else
  {
    parent = path.Dirname();
    name = path.Basename();
  }
  
  return true;
}

Owner OwnerCache::Owner(const RealPath& path)
{
  RealPath parent;
  Path name;
  if (!GetParentName(path, parent, name)) return fs::Owner(0, 0);

  boost::upgrade_lock<boost::shared_mutex> readLock(instance.cacheMutex);
  try
  {
    CacheEntry& entry = instance.cache.Lookup(parent.ToString());
    return entry.first->Owner(name);
  }
  catch (const std::out_of_range&)
  {
    std::unique_ptr<OwnerFile> ownerFile(new OwnerFile(parent));
    if (!ownerFile->Load()) return fs::Owner(0, 0);
    
    fs::Owner owner = ownerFile->Owner(name);
    CacheEntry entry = std::make_pair(ownerFile.release(), true);

    {
      boost::upgrade_to_unique_lock<boost::shared_mutex> writeLock(readLock);
      instance.cache.Insert(parent.ToString(), entry);
    }
    
    return owner;
  }
}

Owners OwnerCache::Owners(const RealPath& parent)
{
  boost::upgrade_lock<boost::shared_mutex> readLock(instance.cacheMutex);
  try
  {
    CacheEntry& entry = instance.cache.Lookup(parent.ToString());
    return entry.first->Owners();
  }
  catch (const std::out_of_range&)
  {
    std::unique_ptr<OwnerFile> ownerFile(new OwnerFile(parent));
    if (!ownerFile->Load()) return fs::Owners();
    CacheEntry entry = std::make_pair(ownerFile.release(), true);

    {
      boost::upgrade_to_unique_lock<boost::shared_mutex> writeLock(readLock);
      instance.cache.Insert(parent.ToString(), entry);
    }

    return entry.first->Owners();
  }
}

void OwnerCache::Delete(const RealPath& path)
{
  RealPath parent;
  Path name;
  if (!GetParentName(path, parent, name)) return;

  boost::upgrade_lock<boost::shared_mutex> readLock(instance.cacheMutex);
  try
  {
    CacheEntry& entry = instance.cache.Lookup(parent.ToString());
    
    boost::upgrade_to_unique_lock<boost::shared_mutex> writeLock(readLock);
    entry.first->Delete(name);
    entry.second = true; // save please!
  }
  catch (const std::out_of_range&)
  {
    std::unique_ptr<OwnerFile> ownerFile(new OwnerFile(parent));
    if (!ownerFile->Load()) return;
    ownerFile->Delete(name);
    CacheEntry entry = std::make_pair(ownerFile.release(), true /* save please */);
    
    boost::upgrade_to_unique_lock<boost::shared_mutex> writeLock(readLock);
    instance.cache.Insert(parent.ToString(), entry);
  }
  
  instance.needSave = true;
  instance.saveCond.notify_one();
}

void OwnerCache::Flush(const RealPath& path)
{
  RealPath parent;
  Path name;
  if (!GetParentName(path, parent, name)) return;

  boost::unique_lock<boost::shared_mutex> writeLock(instance.cacheMutex);
  try
  {
    instance.cache.Flush(parent.ToString());
  }
  catch (const std::out_of_range&)
  { }
}

void OwnerCache::Main()
{
  while (true)
  {
    boost::upgrade_lock<boost::shared_mutex> readLock(cacheMutex);
    if (!needSave) saveCond.wait(readLock);
    
    bool loopUsed = false;
    for (auto& kv : cache)
    {
      if (kv.second.second)
      {
        // this is doing the save with the mutex locked
        // might be better to copy the object, release the
        // lock, then save it, i suspect copying the object
        // would take much less time than serializing it to disk
        kv.second.first->Save();
        
        {
          boost::upgrade_to_unique_lock<boost::shared_mutex> writeLock(readLock);
          kv.second.second = false;
        }
        
        loopUsed = true;
      }
    }
    
    needSave = loopUsed;
  }
}

void OwnerCache::Start()
{
  logs::debug << "Starting owner cache.." << logs::endl;
  instance.thread = boost::thread(std::bind(&OwnerCache::Main, &instance));
}

void OwnerCache::Stop()
{
  logs::debug << "Stopping owner cache.." << logs::endl;
  instance.saveCond.notify_one();
  instance.thread.interrupt();
  instance.thread.join();
}

std::ostream& operator<<(std::ostream& os, const Owner& owner)
{
  os << owner.UID() << "," << owner.GID();
  return os;
}

} /* fs namespace */
