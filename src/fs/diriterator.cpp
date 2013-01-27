#include <cassert>
#include "fs/diriterator.hpp"
#include "ftp/client.hpp"
#include "fs/path.hpp"
#include "acl/path.hpp"
#include "cfg/get.hpp"
#include "util/error.hpp"

namespace fs
{

DirIterator::DirIterator(const Path& path) :
  client(nullptr), path(RealPath(path)), dep(nullptr)
{
  Opendir();
}

DirIterator::DirIterator(const ftp::Client& client, const VirtualPath& path) :
  client(&client), path(MakeReal(path)), dep(nullptr)
{
  Opendir();
}

void DirIterator::Opendir()
{
  namespace PP = acl::path;
  
  if (client)
  {  
    util::Error e = PP::DirAllowed<PP::View>(client->User(), MakeVirtual(path));
    if (!e) throw util::SystemError(e.Errno());
  }

  dp.reset(opendir(path.CString()),closedir);
  if (!dp.get()) throw util::SystemError(errno);
  
  NextEntry();
}

void DirIterator::NextEntry()
{
  namespace PP = acl::path;
  while (true)
  {
    if (readdir_r(dp.get(), &de, &dep) < 0)
      throw util::SystemError(errno);
    if (!dep) break;

    if (!strcmp(de.d_name, ".") ||
        !strcmp(de.d_name, ".."))
        continue;
        
    if (client)
    {
      try
      {
        Status status(path / de.d_name);
        if (status.IsDirectory())
        {
          if (!PP::DirAllowed<PP::View>(client->User(), 
              MakeVirtual(path) / de.d_name)) continue;
        }
        else
        {
          if (!PP::FileAllowed<PP::View>(client->User(), 
              MakeVirtual(path) / de.d_name)) continue;          
        }
      }
      catch (const util::SystemError& e)
      {  continue; }
    }
    
    current = fs::Path(de.d_name);
    break;
  }
}

DirIterator& DirIterator::operator++()
{
  NextEntry();
  return *this;
}

DirIterator& DirIterator::Rewind()
{
  rewinddir(dp.get());
  NextEntry();
  return *this;
}

} /* fs namespace */

#ifdef FS_DIRITERATOR_TEST

#include <iostream>

int main()
{
  using namespace fs;
  
  DirIterator end;
  DirIterator it(".");
  
  for (; it != end; ++it)
  {
    std::cout << *it << std::endl;
  }
  
  try
  {
    DirIterator end;
    DirIterator it("/some/nonexistent/path");
    
    for (; it != end; ++it)
    {
      std::cout << *it << std::endl;
    }    
  }
  catch (const util::SystemError& e)
  {
    std::cout << e.Message() << std::endl;
  }
}

#endif
