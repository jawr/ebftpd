#ifndef __FS_DIRITERATOR_HPP
#define __FS_DIRITERATOR_HPP

#include <iostream>
#include <iterator>
#include <string>
#include <dirent.h>
#include <memory>
#include "fs/path.hpp"

namespace ftp
{
class Client;
}

namespace fs
{

class DirContainer;

class DirIterator : 
  public std::iterator<std::forward_iterator_tag, std::string>
{
  const ftp::Client* client;
  RealPath path;
  struct dirent de;
  struct dirent *dep;
  std::shared_ptr<DIR> dp;
  Path current;
  
  void Opendir();
  void NextEntry();
  DirIterator& Rewind();
  
public:
  explicit DirIterator() : client(nullptr), dep(nullptr) { }
  explicit DirIterator(const Path& path);
  explicit DirIterator(const ftp::Client& client, const VirtualPath& path);
  
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
