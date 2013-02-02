#ifndef __FS_DIRITERATOR_HPP
#define __FS_DIRITERATOR_HPP

#include <iostream>
#include <iterator>
#include <string>
#include <dirent.h>
#include <memory>
#include "fs/path.hpp"

namespace acl
{
class User;
}

namespace fs
{

class DirContainer;

class DirIterator : 
  public std::iterator<std::forward_iterator_tag, std::string>
{
  const acl::User* user;
  RealPath path;
  struct dirent de;
  struct dirent *dep;
  std::shared_ptr<DIR> dp;
  Path current;
  
  void Opendir();
  void NextEntry();
  DirIterator& Rewind();
  
public:
  explicit DirIterator() : user(nullptr), dep(nullptr) { }
  explicit DirIterator(const Path& path);
  explicit DirIterator(const acl::User& user, const VirtualPath& path);
  
  bool operator==(const DirIterator& rhs)
  { return dep == rhs.dep; }
  
  bool operator!=(const DirIterator& rhs)
  { return !operator==(rhs); }
  
  DirIterator& operator++();
  const Path& operator*() const { return current; }
  const Path* operator->() const { return &current; }
  
  friend class DirContainer;
};

} /* fs namespace */

#endif
