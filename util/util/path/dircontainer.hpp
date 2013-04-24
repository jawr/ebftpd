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

#ifndef __UTIL_DIRCONTAINER_HPP
#define __UTIL_DIRCONTAINER_HPP

#include <utility>

namespace util { namespace path
{

template <typename IteratorType>
class GenericDirContainer
{
  IteratorType it;
  IteratorType endIt;
  
public:  
  explicit GenericDirContainer(IteratorType begin) :
    it(begin)
  { }
  
  template <typename... Args>
  explicit GenericDirContainer(Args&&... args) : 
    it(std::forward<Args>(args)...)
  { }

  IteratorType begin()
  {
    it.Rewind();
    return it;
  }
  IteratorType end() { return endIt; }
};

class DirIterator;
typedef GenericDirContainer<path::DirIterator> DirContainer;

class RecursiveDirIterator;
typedef GenericDirContainer<path::RecursiveDirIterator> RecursiveDirContainer;

class GlobIterator;
typedef GenericDirContainer<path::GlobIterator> GlobDirContainer;

} /* path namespace */
} /* util namespace */

#endif
