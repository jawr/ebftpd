#include <cassert>
#include <memory>
#include <cstring>
#include <dirent.h>
#include "fs/direnumerator.hpp"
#include "fs/status.hpp"
#include "ftp/client.hpp"
#include "acl/path.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "logs/logs.hpp"

namespace fs
{

DirEnumerator::DirEnumerator() :
  client(nullptr),
  totalBytes(0),
  loadOwners(true)
{
}

DirEnumerator::DirEnumerator(const fs::Path& path, bool loadOwners) :
  client(nullptr),
  path(path),
  totalBytes(0),
  loadOwners(loadOwners)
{
  Readdir();
}

DirEnumerator::DirEnumerator(ftp::Client& client, const fs::VirtualPath& path, bool loadOwners) :
  client(&client),
  path(MakeReal(path)),
  totalBytes(0),
  loadOwners(loadOwners)
{
  Readdir();
}

void DirEnumerator::Readdir(const fs::Path& path, bool loadOwners)
{
  this->path = RealPath(path);
  this->loadOwners = loadOwners;
  Readdir();
}

void DirEnumerator::Readdir(ftp::Client& client, const fs::VirtualPath& path, bool loadOwners)
{
  this->client  = &client;
  this->path = MakeReal(path);
  this->loadOwners = loadOwners;
  Readdir();
}

void DirEnumerator::Readdir()
{
  namespace PP = acl::path;

  if (client && !PP::DirAllowed<PP::View>(client->User(), 
      MakeVirtual(path))) 
  {
    return;
  }

  std::shared_ptr<DIR> dp(opendir(path.CString()), closedir);
  if (!dp.get()) throw util::SystemError(errno);
  
  struct dirent de;
  struct dirent* dep;
  while (true)
  {
    readdir_r(dp.get(), &de, &dep);
    if (!dep) break;

     if (!strcmp(de.d_name, ".") || !strcmp(de.d_name, "..")) continue;

    fs::RealPath entryPath(path / de.d_name);

    try
    {
      fs::Status status(entryPath);
      totalBytes += status.Size();
      
      if (client)
      {
        fs::VirtualPath virtPath(MakeVirtual(entryPath));
        util::Error hideOwner;
        if (status.IsDirectory())
        {
          if (!PP::DirAllowed<PP::View>(client->User(), virtPath)) continue;
          hideOwner = PP::DirAllowed<PP::Hideowner>(client->User(), virtPath);
        }
        else
        {
          if (!PP::FileAllowed<PP::View>(client->User(), virtPath)) continue;
          hideOwner = PP::FileAllowed<PP::Hideowner>(client->User(), virtPath);
        }
        
        Owner owner(0, 0);
        if (!hideOwner && loadOwners) owner = OwnerCache::Owner(entryPath);
        entries.emplace_back(fs::Path(de.d_name), status, owner);
      }
      else
      {
        Owner owner(0, 0);
        if (loadOwners) owner = OwnerCache::Owner(entryPath);
        entries.emplace_back(fs::Path(de.d_name), status, owner);
      }
    }
    catch (const util::SystemError&)
    {
      continue;
    }
  }
}

} /* fs namespace */

#ifdef FS_DIRENUMERATOR_TEST

#include <iostream>

int main()
{
  using namespace fs;
  
  DirEnumerator dirEnum("/tmp");
  
  std::cout << "unsorted" << std::endl;
  
  for (DirEnumerator::const_iterator it =
       dirEnum.begin(); it != dirEnum.end(); ++it)
  {
    std::cout << it->Path() << std::endl;
  }
  
  std::cout << "path asc" << std::endl;
  
  std::sort(dirEnum.begin(), dirEnum.end(), DirEntryPathLess());
  
  for (DirEnumerator::const_iterator it =
       dirEnum.begin(); it != dirEnum.end(); ++it)
  {
    std::cout << it->Path() << std::endl;
  }
  
  std::cout << "path desc" << std::endl;
  
  std::sort(dirEnum.begin(), dirEnum.end(), DirEntryPathGreater());
  
  for (DirEnumerator::const_iterator it =
       dirEnum.begin(); it != dirEnum.end(); ++it)
  {
    std::cout << it->Path() << std::endl;
  }
  
  std::cout << "size asc" << std::endl;
  
  std::sort(dirEnum.begin(), dirEnum.end(), DirEntrySizeLess());
  
  for (DirEnumerator::const_iterator it =
       dirEnum.begin(); it != dirEnum.end(); ++it)
  {
    std::cout << it->Path() << std::endl;
  }

  std::cout << "size desc" << std::endl;
  
  std::sort(dirEnum.begin(), dirEnum.end(), DirEntrySizeGreater());
  
  for (DirEnumerator::const_iterator it =
       dirEnum.begin(); it != dirEnum.end(); ++it)
  {
    std::cout << it->Path() << std::endl;
  }

  std::cout << "mod time asc" << std::endl;
  
  std::sort(dirEnum.begin(), dirEnum.end(), DirEntryModTimeLess());
  
  for (DirEnumerator::const_iterator it =
       dirEnum.begin(); it != dirEnum.end(); ++it)
  {
    std::cout << it->Path() << std::endl;
  }
  
  std::cout << "mod time desc" << std::endl;
  
  std::sort(dirEnum.begin(), dirEnum.end(), DirEntryModTimeGreater());
  
  for (DirEnumerator::const_iterator it =
       dirEnum.begin(); it != dirEnum.end(); ++it)
  {
    std::cout << it->Path() << std::endl;
  }


}

#endif
