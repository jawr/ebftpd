#include <fstream>
#if defined(TEXT_OWNER_FILES)
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#else
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif
#include "owner.hpp"
#include "status.hpp"
#include "exception.hpp"

namespace fs
{

const std::string OwnerFile::ownerFilename = ".owner";

void OwnerFile::Create(const std::string& name, const class Owner& owner)
{
  entries.insert(std::make_pair(name, OwnerEntry(name, owner)));
}

void OwnerFile::Chown(const std::string& name, const class Owner& owner)
{
  entries.at(name).Chown(owner);
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

bool OwnerFile::Load()
{
  try
  {
    Status status(ownerFile);
    if (!status.IsRegularFile()) return false;
  }
  catch (const FileSystemError&)
  {
    return false;
  }
  
  std::ifstream fin(ownerFile.c_str());
  if (!fin) return false;
  
#if defined(TEXT_OWNER_FILES)
  boost::archive::text_iarchive ia(fin);
#else
  boost::archive::binary_iarchive ia(fin);
#endif
  ia >> *this;
  return fin;
}

bool OwnerFile::Save()
{
  std::ofstream fout(ownerFile.c_str());
  if (!fout) return false;
  
#if defined(TEXT_OWNER_FILES)
  boost::archive::text_oarchive oa(fout);
#else
  boost::archive::binary_oarchive oa(fout);
#endif
  oa << *this;
  return fout;
}

} /* fs namespace */


#ifdef FS_OWNER_TEST

#include <boost/archive/text_oarchive.hpp>

int main()
{
  fs::OwnerFile of("/home/bioboy");
  of.Create("test1", fs::Owner(1, 1));
  of.Create("test2", fs::Owner(2, 2));
  of.Save();
  
  std::cout << "test 2 UID: " << of.Owner("test2").UID() << std::endl;
  std::cout << "test 1 GID: " << of.Owner("test1").GID() << std::endl;
  std::cout << "test 1 exists: " << of.Exists("test1") << std::endl;
  of.Chown("test1", fs::Owner(69, 69));
  std::cout << "test 1 UID, GID: " << of.Owner("test1").UID() << ", " << of.Owner("test1").GID() << std::endl;
  of.Rename("test1", "test69");
  std::cout << "test 1 exists: " << of.Exists("test1") << std::endl;
  std::cout << "test 69 exists: " << of.Exists("test69") << std::endl;
  of.Delete("test2");
  std::cout << "test 2 exists: " << of.Exists("test2") << std::endl;
  
  of.Save();
  
  fs::OwnerFile of2("/home/bioboy");
  of2.Load();
  boost::archive::text_oarchive oa(std::cout);  
  oa << of2;
}

#endif
