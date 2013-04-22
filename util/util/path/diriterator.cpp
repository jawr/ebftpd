//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
  DIR* dp = opendir(path.c_str());
  if (!dp) throw util::SystemError(errno);
  
  this->dp.reset(dp, closedir);
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
