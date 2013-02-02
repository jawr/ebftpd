#include <cassert>
#include "fs/diriterator.hpp"
#include "acl/user.hpp"
#include "fs/path.hpp"
#include "acl/path.hpp"
#include "util/error.hpp"
#include "fs/status.hpp"

namespace fs
{

DirIterator::DirIterator(const Path& path) :
  user(nullptr), path(RealPath(path)), dep(nullptr)
{
  Opendir();
}

DirIterator::DirIterator(const acl::User& user, const VirtualPath& path) :
  user(&user), path(MakeReal(path)), dep(nullptr)
{
  Opendir();
}

void DirIterator::Opendir()
{
  namespace PP = acl::path;
  
  if (user)
  {  
    util::Error e = PP::DirAllowed<PP::View>(*user, MakeVirtual(path));
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
        
    if (user)
    {
      try
      {
        Status status(path / de.d_name);
        if (status.IsDirectory())
        {
          if (!PP::DirAllowed<PP::View>(*user, 
              MakeVirtual(path) / de.d_name)) continue;
        }
        else
        {
          if (!PP::FileAllowed<PP::View>(*user, 
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
