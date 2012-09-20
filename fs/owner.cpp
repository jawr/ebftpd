#include <fstream>
#if defined(TEXT_OWNER_FILES)
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#else
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif
#include "fs/owner.hpp"
#include "fs/status.hpp"
#include "util/error.hpp"
#include "logger/logger.hpp"

namespace fs
{

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

void OwnerFile::Rename(const std::string& oldName, const std::string& newName)
{
  boost::unordered_map<std::string, OwnerEntry>::iterator it;
  it = entries.find(oldName);
  if (it == entries.end()) throw std::out_of_range("key doesn't exist");
  it->second.Rename(newName);
  entries.insert(std::make_pair(newName, it->second));
  entries.erase(it);
}

void OwnerFile::Delete(const std::string& name)
{
  entries.erase(name);
}

bool OwnerFile::Exists(const std::string& name) const
{
  return entries.find(name) != entries.end();
}

const class Owner& OwnerFile::GetOwner(const std::string& name) const
{
  return entries.at(name).GetOwner();
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
    return e.ValidErrno() && e.Errno() == ENOENT;
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

OwnerModify::OwnerModify(const Path& path) :
  path(path),
  parent(path.Dirname()),
  name(path.Basename()),
  ownerFile(parent) 
{
}

void OwnerModify::Chown(const Owner& owner)
{
  if (!ownerFile.Load()) return;
  ownerFile.Chown(name, owner);
  ownerFile.Save();
}

void OwnerModify::Rename(const Path& newName)
{
  if (!ownerFile.Load()) return;
  if (!ownerFile.Exists(name)) return;
  ownerFile.Rename(name, newName);
  ownerFile.Save();
}

void OwnerModify::Delete()
{
  if (!ownerFile.Load()) return;
  if (!ownerFile.Exists(name)) return;
  ownerFile.Delete(name);
  ownerFile.Save();
}

Owner GetOwner(const Path& path)
{
  OwnerFile ownerFile(path.Dirname());
  if (!ownerFile.Load()) return Owner();
  
  std::string name = path.Basename();
  if (!ownerFile.Exists(name)) return Owner();
  return ownerFile.GetOwner(name);
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
/*
  try
  {
    std::cout << "getowner: " << GetOwner("/tmp/somefile") << std::endl;
  }
  catch (const util::SystemError& e)
  {
    std::cout << "getowner: " << e.what() << std::endl;
  }
  
  system("touch /tmp/somefile");
  OwnerModify("/tmp/somefile").Chown(Owner(69, 69));
  OwnerModify("/tmp/somefile").Rename("otherfile");
  OwnerModify("/tmp/otherfile").Delete();
*/  
  boost::posix_time::ptime start(boost::posix_time::microsec_clock::local_time());
  for (int i = 0; i < 100; ++i)
  {
    std::stringstream test;
    test << "/tmp/" << i;
    //system(("touch " + test.str()).c_str());
    OwnerModify(test.str()).Chown(Owner(i, i));
    boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());
    std::cout << 1 << " " << (end - start).total_microseconds() << std::endl;
  }
  boost::posix_time::ptime end(boost::posix_time::microsec_clock::local_time());
  std::cout << (end - start).total_microseconds() << std::endl;
/*
  for (int i = 0; i < 100; ++i)
  {
    std::stringstream test;
    test << "/tmp/" << i;
    std::cout << "getowner " << i << ": " << GetOwner(test.str()) << std::endl;
  }
  
  OwnerFile of("/home/bioboy");
  of.Create("test1", Owner(1, 1));
  of.Create("test2", Owner(2, 2));
  of.Save();
  
  std::cout << "test 2 UID: " << of.GetOwner("test2").UID() << std::endl;
  std::cout << "test 1 GID: " << of.GetOwner("test1").GID() << std::endl;
  std::cout << "test 1 exists: " << of.Exists("test1") << std::endl;
  of.Chown("test1", Owner(69, 69));
  std::cout << "test 1 UID, GID: " << of.GetOwner("test1").UID() << ", " << of.GetOwner("test1").GID() << std::endl;
  of.Rename("test1", "test69");
  std::cout << "test 1 exists: " << of.Exists("test1") << std::endl;
  std::cout << "test 69 exists: " << of.Exists("test69") << std::endl;
  of.Delete("test2");
  std::cout << "test 2 exists: " << of.Exists("test2") << std::endl;
  
  of.Save();
  
  OwnerFile of2("/home/bioboy");
  of2.Load();
  boost::archive::text_oarchive oa(std::cout);  
  oa << of2;*/
}

#endif
