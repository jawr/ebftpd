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
  
  virtual util::Error Check(const std::string& /* path */) { return util::Error::Success(); }

protected:
  std::function<bool(std::string)> filter;
  std::string current;

  virtual std::string NextEntry();
  
public:
  DirIterator() : dep(nullptr) { }
  explicit DirIterator(const std::string& path, bool basenameOnly = true);
  explicit DirIterator(const std::string& path, const std::function<bool(std::string)>& filter, 
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
