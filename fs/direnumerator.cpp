#include <tr1/memory>
#include <dirent.h>
#include "fs/direnumerator.hpp"
#include "fs/status.hpp"
#include "ftp/client.hpp"

namespace fs
{

DirEnumerator::DirEnumerator() :
  client(0)
{
}

DirEnumerator::DirEnumerator(const fs::Path& path) :
  client(0),
  path(path)
{
  Readdir();
}

DirEnumerator::DirEnumerator(ftp::Client& client, const fs::Path& path) :
  client(&client),
  path(path)
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
  std::tr1::shared_ptr<DIR> dp(opendir(path.CString()), closedir);
  if (!dp.get()) return;
  
  struct dirent de;
  struct dirent* dep;
  while (true)
  {
    readdir_r(dp.get(), &de, &dep);
    if (!dep) break;
    
    fs::Path fullPath(path);
    fullPath /= de.d_name;
    
    
    if (client)
    {
      // apply ACLs to hide privpaths or 
      // other hidden directories / files
    }
    
    try
    {
      fs::Status status(fullPath);
      entries.push_back(new DirEntry(de.d_name, status));
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
