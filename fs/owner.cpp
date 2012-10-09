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
#include "logger/logger.hpp"
#include "fs/status.hpp"

namespace fs
{

OwnerCache OwnerCache::instance;
const std::string OwnerFile::ownerFilename = ".owner";

void OwnerFile::Create(const std::string& name, const class Owner& owner)
{
  entries.insert(std::make_pair(name, OwnerEntry(name, owner)));
}

void OwnerFile::Chown(const std::string& name, const class Owner& owner)
{
  if (!Exists(name)) Create(name, owner);
  else entries.at(name).Chown(owner);
}

void OwnerFile::Delete(const std::string& name)
{
  entries.erase(name);
}

bool OwnerFile::Exists(const std::string& name) const
{
  return entries.find(name) != entries.end();
}

class fs::Owner OwnerFile::Owner(const std::string& name) const
{
  if (Exists(name)) return entries.at(name).Owner();
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
  
  std::ifstream fin(ownerFile.c_str());
  if (!fin) return false;
  
  try
  {
    lock = FileLock::Create(ownerFile);
  }
  catch (const util::SystemError&)
  {
    return false;
  }
  
#if defined(TEXT_OWNER_FILES)
  boost::archive::text_iarchive ia(fin);
#else
  boost::archive::binary_iarchive ia(fin);
#endif
  ia >> *this;
  return fin;
}

bool OwnerFile::Load(FileLockPtr& lock)
{
  if (!InnerLoad(lock))
  {
    logger::error << "Unable to load owner file: " << ownerFile << logger::endl;
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
  std::ofstream fout(ownerFile.c_str());
  if (!fout) return false;
  
  if (!lock.get())
  {
    try
    {
      lock = FileLock::Create(ownerFile);
    }
    catch(const util::SystemError&)
    {
      return false;
    }
  }
  
#if defined(TEXT_OWNER_FILES)
  boost::archive::text_oarchive oa(fout);
#else
  boost::archive::binary_oarchive oa(fout);
#endif
  oa << *this;
  return fout;
}

bool OwnerFile::Save(FileLockPtr& lock)
{
  if (!InnerSave(lock))
  {
    logger::error << "Unable to save owner file: " << ownerFile << logger::endl;
    return false;
  }
  return true;
}

bool OwnerFile::Save()
{
  FileLockPtr lock;
  return Save(lock);
}

void OwnerCache::Chown(const fs::Path& path, const fs::Owner& owner)
{
  fs::Path parent;
  fs::Path name;
  if (!GetParentName(path, parent, name)) return;

  boost::unique_lock<boost::shared_mutex> readLock(instance.cacheMutex);
  try
  {
    CacheEntry& entry = instance.cache.Lookup(parent);
    entry.first->Chown(name, owner);
    entry.second = true; // save please!
  }
  catch (const std::out_of_range&)
  {
    std::unique_ptr<OwnerFile> ownerFile(new OwnerFile(parent));
    if (!ownerFile->Load()) return;
    ownerFile->Chown(name, owner);
    CacheEntry entry = std::make_pair(ownerFile.release(), true /* save please */);
    instance.cache.Insert(parent, entry);
  }
  
  instance.needSave = true;
  instance.saveCond.notify_one();
}

bool OwnerCache::GetParentName(const fs::Path& path, fs::Path& parent, fs::Path& name)
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
    name = ".";
  }
  else
  {
    parent = path.Dirname();
    name = path.Basename();
  }
  
  return true;
}

Owner OwnerCache::Owner(const fs::Path& path)
{
  fs::Path parent;
  fs::Path name;
  if (!GetParentName(path, parent, name)) return fs::Owner(0, 0);

  boost::shared_lock<boost::shared_mutex> readLock(instance.cacheMutex);
  try
  {
    CacheEntry& entry = instance.cache.Lookup(parent);
    return entry.first->Owner(name);
  }
  catch (const std::out_of_range&)
  {
    std::unique_ptr<OwnerFile> ownerFile(new OwnerFile(parent));
    if (!ownerFile->Load()) return fs::Owner(0, 0);
    
    fs::Owner owner = ownerFile->Owner(name);
    CacheEntry entry = std::make_pair(ownerFile.release(), true);

    boost::upgrade_lock<boost::shared_mutex> writeLock(instance.cacheMutex);
    instance.cache.Insert(parent, entry);
    return owner;
  }
}

void OwnerCache::Delete(const Path& path)
{
  fs::Path parent;
  fs::Path name;
  if (!GetParentName(path, parent, name)) return;

  boost::unique_lock<boost::shared_mutex> readLock(instance.cacheMutex);
  try
  {
    CacheEntry& entry = instance.cache.Lookup(parent);
    entry.first->Delete(name);
    entry.second = true; // save please!
  }
  catch (const std::out_of_range&)
  {
    std::unique_ptr<OwnerFile> ownerFile(new OwnerFile(parent));
    if (!ownerFile->Load()) return;
    ownerFile->Delete(name);
    CacheEntry entry = std::make_pair(ownerFile.release(), true /* save please */);
    instance.cache.Insert(parent, entry);
  }
  
  instance.needSave = true;
  instance.saveCond.notify_one();
}

void OwnerCache::Main()
{
  while (true)
  {
    boost::shared_lock<boost::shared_mutex> lock(cacheMutex);
    if (!needSave)
    {
      boost::this_thread::interruption_point();
      saveCond.wait(lock);
    }
    
    bool loopUsed = false;
    for (auto& kv : cache)
    {
      if (kv.second.second)
      {
        kv.second.first->Save();
        kv.second.second = false;
        loopUsed = true;
      }
    }
    
    needSave = loopUsed;
  }
}

void OwnerCache::Start()
{
  instance.thread = boost::thread(std::bind(&OwnerCache::Main, &instance));
}

void OwnerCache::Stop()
{
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

#ifdef FS_OWNER_TEST

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <iostream>
#include <cstdlib>
#include <sstream>

int main()
{
  using namespace fs;
  using namespace boost::posix_time;

  OwnerCache::Start();
  
  {

    boost::posix_time::ptime start(boost::posix_time::microsec_clock::local_time());
    for (int i = 0; i < 1000; ++i)
    {
      start = boost::posix_time::microsec_clock::local_time();
      std::stringstream test;
      test << "/tmp/" << i;
      OwnerCache::Chown(test.str(), Owner(i, i));
      boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());
      //std::cout << i << " " << (end - start).total_microseconds() << std::endl;
    }
    boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());
    std::cout << (end - start).total_microseconds() << std::endl;
  }
  
  {
    boost::posix_time::ptime start(boost::posix_time::microsec_clock::local_time());
    for (int i = 0; i < 100; ++i)
    {
      start = boost::posix_time::microsec_clock::local_time();
      std::stringstream test;
      test << "/tmp/" << i;
      /*std::cout << "owner: " << */OwnerCache::Owner(test.str())/* << std::endl*/;
      boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());
      std::cout << i << " " << (end - start).total_microseconds() << std::endl;
    }
    boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());
    std::cout << (end - start).total_microseconds() << std::endl;
  }
  
  {
    boost::posix_time::ptime start(boost::posix_time::microsec_clock::local_time());
    for (int i = 0; i < 1000; ++i)
    {
      start = boost::posix_time::microsec_clock::local_time();
      std::stringstream test;
      test << "/tmp/" << i;
      OwnerCache::Delete(test.str());
      boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());
      std::cout << i << " " << (end - start).total_microseconds() << std::endl;
    }
    boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());
    std::cout << (end - start).total_microseconds() << std::endl;
  }
  
  OwnerCache::Stop();
}

#endif
