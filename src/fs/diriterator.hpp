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

#ifndef __FS_DIRITERATOR_HPP
#define __FS_DIRITERATOR_HPP

#include "fs/path.hpp"
#include "util/path/diriterator.hpp"

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

class DirIterator : public util::path::DirIterator
{
public:
  DirIterator() = default;
  DirIterator(const acl::User& user, const VirtualPath& path, ValueType valueType = BasenameOnly);
};

typedef util::path::GenericDirContainer<DirIterator> DirContainer;

} /* fs namespace */

#endif
