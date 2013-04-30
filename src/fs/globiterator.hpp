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

#ifndef __FS_GLOBITERATOR_HPP
#define __FS_GLOBITERATOR_HPP

#include "fs/path.hpp"
#include "util/path/globiterator.hpp"

namespace acl
{
class User;
}

namespace util { namespace path
{
template <typename IteratorType> class GenericDirContainer;
}
}

namespace fs
{

class GlobIterator : public util::path::GlobIterator
{
  mutable std::string current;
  
  std::string& Current() const
  {
    if (current.empty())
    {
      current = MakeVirtual(fs::RealPath(util::path::GlobIterator::operator*())).ToString();
    }
    return current;
  }
  
public:
  GlobIterator() = default;
  GlobIterator(const acl::User& user, const VirtualPath& path, bool recursive = false);
  
  GlobIterator& operator++()
  {
    current.clear();
    util::path::GlobIterator::operator++();
    return *this;
  }
  
  const std::string& operator*() const
  {
    return Current();
  }
  const std::string* operator->() const
  {
    return &Current();
  }
};

typedef util::path::GenericDirContainer<GlobIterator> GlobContainer;

} /* fs namespace */

#endif
