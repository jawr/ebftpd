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
#include <boost/filesystem.hpp>
#include "util/path/recursivediriterator.hpp"
#include "util/error.hpp"

namespace util { namespace path
{

namespace fs = boost::filesystem3;

RecursiveDirIterator::RecursiveDirIterator() :
  iter(new fs::recursive_directory_iterator())
{
}

RecursiveDirIterator::RecursiveDirIterator(const std::string& path) :
  path(path)
{
  OpenDirectory();
}

RecursiveDirIterator::RecursiveDirIterator(const std::string& path, 
            const std::function<bool(const std::string&)>& filter) :
  path(path),
  filter(filter)
{
  OpenDirectory();
  if (filter)
  {
    RecursiveDirIterator end;
    while (*this != end && filter(**this))
    {
      ++(*this);
    }
  }
}

void RecursiveDirIterator::OpenDirectory()
{
  try
  {
    iter.reset(new fs::recursive_directory_iterator(path));
  }
  catch (const fs::filesystem_error& e)
  {
    throw util::SystemError(e.code().value());
  }
}

RecursiveDirIterator::~RecursiveDirIterator()
{
}  

RecursiveDirIterator& RecursiveDirIterator::Rewind()
{
  OpenDirectory();
  return *this;
}

bool RecursiveDirIterator::operator==(const DirIteratorBase& rhs)
{
  assert(dynamic_cast<const RecursiveDirIterator*>(&rhs));
  return *this->iter ==
         *reinterpret_cast<const RecursiveDirIterator*>(&rhs)->iter;
}

bool RecursiveDirIterator::operator!=(const DirIteratorBase& rhs)
{
  return !operator==(rhs);
}

RecursiveDirIterator& RecursiveDirIterator::operator++()
{
  try
  {
    if (filter)
    {
      fs::recursive_directory_iterator end;
      do
      {
        ++(*iter);
      }
      while (*iter != end && filter(**this));
    }
    else
    {
      ++(*iter);
    }
  }
  catch (const fs::filesystem_error& e)
  {
    throw util::SystemError(e.code().value());
  }
  return *this;
}

const std::string& RecursiveDirIterator::operator*() const
{
  current = (*iter)->path().string();
  return current;
}

const std::string* RecursiveDirIterator::operator->() const
{
  current = (*iter)->path().string();
  return &current;
}
  
} /* path namespace */
} /* util namespace */
