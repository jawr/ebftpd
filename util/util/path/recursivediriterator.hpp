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

#ifndef __UTIL_PATH_RECURSIVE_DIRITERATOR_HPP
#define __UTIL_PATH_RECURSIVE_DIRITERATOR_HPP

#include <boost/optional.hpp>
#include "util/path/diriterator.hpp"
#include "util/path/status.hpp"

namespace util { namespace path
{

class RecursiveDirIterator;

class RecursiveDirIterator : public DirIterator
{
  std::shared_ptr<RecursiveDirIterator> subIt;
  std::shared_ptr<RecursiveDirIterator> subEnd;
  bool ignoreErrors;
  
  std::string NextEntry();
  
public:
  RecursiveDirIterator() = default;
  RecursiveDirIterator(const std::string& path, bool ignoreErrors = false) : 
    DirIterator(path, false), 
    ignoreErrors(ignoreErrors)
  { }

  RecursiveDirIterator(const std::string& path, const std::function<bool(const 
                       std::string&)>& filter, bool ignoreErrors = false) : 
    DirIterator(path, filter, false), 
    ignoreErrors(ignoreErrors)
  { }
  

  RecursiveDirIterator& Rewind()
  {
    subIt = nullptr;
    DirIterator::Rewind();
    return *this;
  }
  
  bool operator==(const RecursiveDirIterator& rhs)
  {
    if (subIt)
    {
      if (!rhs.subIt)
      {
        return false;
      }
      
      if (*subIt != *rhs.subIt)
      {
        return false;
      }
    }
    else
    if (rhs.subIt)
    {
      return false;
    }
    return DirIterator::operator==(rhs);
  }
  
  bool operator!=(const RecursiveDirIterator& rhs)
  { return !operator==(rhs); }
};

} /* path namespace */
} /* util namespace */

#endif
