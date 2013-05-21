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
#include "util/path/diriterator.hpp"
#include "util/error.hpp"

namespace util { namespace path
{

namespace fs = boost::filesystem3;

DirIterator::DirIterator() :
  iter(new fs::directory_iterator())
{
}

DirIterator::DirIterator(const std::string& path, ValueType valueType) :
  path(path),
  valueType(valueType)
{
  OpenDirectory();
}

DirIterator::DirIterator(const std::string& path, 
            const std::function<bool(const std::string&)>& filter,
            ValueType valueType) :
  path(path),
  filter(filter),
  valueType(valueType)
{
  OpenDirectory();
  if (filter)
  {
    DirIterator end;
    while (*this != end && filter(**this))
    {
      ++(*this);
    }
  }
}

void DirIterator::OpenDirectory()
{
  try
  {
    iter.reset(new fs::directory_iterator(path));
  }
  catch (const fs::filesystem_error& e)
  {
    throw util::SystemError(e.code().value());
  }
}

DirIterator::~DirIterator()
{
}  

DirIterator& DirIterator::Rewind()
{
  OpenDirectory();
  return *this;
}

bool DirIterator::operator==(const DirIteratorBase& rhs)
{
  assert(dynamic_cast<const DirIterator*>(&rhs));
  return *this->iter == *reinterpret_cast<const DirIterator*>(&rhs)->iter;
}

bool DirIterator::operator!=(const DirIteratorBase& rhs)
{
  return !operator==(rhs);
}

DirIterator& DirIterator::operator++()
{
  try
  {
    if (filter)
    {
      fs::directory_iterator end;
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

std::string& DirIterator::Current() const
{
  switch (valueType)
  {
    case AbsolutePath :
      current = (*iter)->path().string();
      break;
    case BasenameOnly :
      current = (*iter)->path().filename().string();
      break;
    default           :
      assert(false);
  }
  
  return current;
}

const std::string& DirIterator::operator*() const
{
  return Current();
}

const std::string* DirIterator::operator->() const
{
  return &Current();
}
  
} /* path namespace */
} /* util namespace */
