#ifndef __UTIL_DIRITERATOR_HPP
#define __UTIL_DIRITERATOR_HPP

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

namespace util
{

class DirContainer;

class DirIterator : 
  public std::iterator<std::forward_iterator_tag, std::string>
{
  std::string path;
  struct dirent de;
  struct dirent *dep;
  std::shared_ptr<DIR> dp;
  std::string current;
  bool basenameOnly;
  
  void Opendir();
  void NextEntry();
  
  virtual util::Error Check(const std::string& /* path */) { return util::Error::Success(); }
  
public:
  DirIterator() : dep(nullptr) { }
  explicit DirIterator(const std::string& path, bool basenameOnly = true);
  
  virtual ~DirIterator() { }
  
  DirIterator& Rewind();

  bool operator==(const DirIterator& rhs)
  { return dep == rhs.dep; }
  
  bool operator!=(const DirIterator& rhs)
  { return !operator==(rhs); }
  
  DirIterator& operator++();
  const std::string& operator*() const { return current; }
  const std::string* operator->() const { return &current; }
  
  friend class DirContainer;
};

} /* util namespace */

#endif
