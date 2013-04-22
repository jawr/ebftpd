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

#ifndef __UTIL_DIRITERATOR_HPP
#define __UTIL_DIRITERATOR_HPP

#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <dirent.h>
#include <memory>
#include "util/error.hpp"

namespace acl
{
class User;
}

namespace util { namespace path
{

class DirIterator : 
  public std::iterator<std::forward_iterator_tag, std::string>
{
  std::string path;
  struct dirent de;
  struct dirent *dep;
  std::shared_ptr<DIR> dp;
  bool basenameOnly;
  
  void Opendir();
  
protected:
  std::function<bool(const std::string&)> filter;
  std::string current;

  virtual std::string NextEntry();
  
public:
  DirIterator() : dep(nullptr) { }
  explicit DirIterator(const std::string& path, bool basenameOnly = true);
  explicit DirIterator(const std::string& path, 
                       const std::function<bool(const std::string&)>& filter, 
                       bool basenameOnly = true);
  
  virtual ~DirIterator() { }
  
  virtual DirIterator& Rewind();

  virtual bool operator==(const DirIterator& rhs)
  { return dep == rhs.dep; }
  
  virtual bool operator!=(const DirIterator& rhs)
  { return !operator==(rhs); }
  
  DirIterator& operator++();
  const std::string& operator*() const { return current; }
  const std::string* operator->() const { return &current; }
};

} /* path namespace */
} /* util namespace */

#endif
