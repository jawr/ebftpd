#include <cstring>
#include <cassert>
#include "util/diriterator.hpp"
#include "util/path.hpp"

namespace util
{

DirIterator::DirIterator(const std::string& path, bool basenameOnly) :
  path(path), dep(nullptr), basenameOnly(basenameOnly)
{
  Opendir();
}

void DirIterator::Opendir()
{
  auto e = Check(path);
  if (!e) throw util::SystemError(e.Errno());

  dp.reset(opendir(path.c_str()), closedir);
  if (!dp.get()) throw util::SystemError(errno);
  
  NextEntry();
}

void DirIterator::NextEntry()
{
  while (true)
  {
    if (readdir_r(dp.get(), &de, &dep) < 0)
      throw util::SystemError(errno);
    if (!dep) break;

    if (!strcmp(de.d_name, ".") ||
        !strcmp(de.d_name, "..") ||
        !Check(util::path::Join(path, de.d_name)))
        continue;
    
    if (!basenameOnly) current = util::path::Join(path, de.d_name);
    else current = de.d_name;
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

} /* util namespace */
