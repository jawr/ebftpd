#include <cassert>
#include <tr1/memory>
#include <cstring>
#include <dirent.h>
#include "fs/direnumerator.hpp"
#include "fs/status.hpp"
#include "ftp/client.hpp"
#include "acl/check.hpp"

#include <iostream>

extern const fs::Path dummySiteRoot;

namespace fs
{

DirEnumerator::DirEnumerator() :
  client(0),
  totalBytes(0)
{
}

DirEnumerator::DirEnumerator(const fs::Path& path) :
  client(0),
  path(path),
  totalBytes(0)
{
  Readdir();
}

DirEnumerator::DirEnumerator(ftp::Client& client, const fs::Path& path) :
  client(&client),
  path(path),
  totalBytes(0)
{
  Readdir();
}

void DirEnumerator::Readdir(const fs::Path& path)
{
  this->path = path;
  Readdir();
}

void DirEnumerator::Readdir(ftp::Client& client, const fs::Path& path)
{
  this->client  = &client;
  this->path = path;
  Readdir();
}

void DirEnumerator::Readdir()
{
  namespace PP = acl::PathPermission;
  
  Path real(path);
  std::cout << real << std::endl;
  if (client)
  {  
    Path absolute = (client->WorkDir() / path).Expand();
    if (!PP::DirAllowed<PP::View>(client->User(), absolute)) return;
    real = dummySiteRoot + absolute;
  }

  std::tr1::shared_ptr<DIR> dp(opendir(real.CString()), closedir);
  if (!dp.get()) throw util::SystemError(errno);
  
  size_t siteRootLen = dummySiteRoot.ToString().length();
  struct dirent de;
  struct dirent* dep;
  while (true)
  {
    readdir_r(dp.get(), &de, &dep);
    if (!dep) break;

     if (!strcmp(de.d_name, ".") || !strcmp(de.d_name, "..")) continue;
    
    fs::Path fullPath(real);
    fullPath /= de.d_name;
        
    try
    {
      fs::Status status(fullPath);
      
      if (client)
      {
        fs::Path absolute = fullPath.ToString().substr(siteRootLen);
        util::Error e;
        if (status.IsDirectory())
          e = PP::DirAllowed<PP::View>(client->User(), absolute);
        else
          e = PP::FileAllowed<PP::View>(client->User(), absolute);
        if (!e) continue;
      }
      
      totalBytes += status.Size();
      fs::Owner owner(OwnerCache::Owner(fullPath));
      entries.push_back(new DirEntry(de.d_name, status, owner));
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
