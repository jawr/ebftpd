#include <cstring>
#include <cassert>
#include "util/path/diriterator.hpp"
#include "util/path/path.hpp"

namespace util { namespace path
{

DirIterator::DirIterator(const std::string& path, bool basenameOnly) :
  path(path), dep(nullptr), basenameOnly(basenameOnly)
{
  Opendir();
}

DirIterator::DirIterator(const std::string& path, 
    const std::function<bool(const std::string&)>& filter, bool basenameOnly) :
  path(path), dep(nullptr), basenameOnly(basenameOnly), filter(filter)
{
  Opendir();
}

void DirIterator::Opendir()
{
  dp.reset(opendir(path.c_str()), closedir);
  if (!dp.get()) throw util::SystemError(errno);
  
  current = NextEntry();
}

std::string DirIterator::NextEntry()
{
  std::string entry;
  while (true)
  {
    if (readdir_r(dp.get(), &de, &dep) < 0)
      throw util::SystemError(errno);
    if (!dep) break;

    if (!strcmp(de.d_name, ".") ||
        !strcmp(de.d_name, "..") ||
        (filter && !filter(util::path::Join(path, de.d_name))))
        continue;
    
    if (!basenameOnly) entry = util::path::Join(path, de.d_name);
    else entry = de.d_name;
    break;
  }
  
  return entry;
}

DirIterator& DirIterator::operator++()
{
  current = NextEntry();
  return *this;
}

DirIterator& DirIterator::Rewind()
{
  rewinddir(dp.get());
  NextEntry();
  return *this;
}

} /* path namespace */
} /* util namespace */
