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
  explicit RecursiveDirIterator(const std::string& path, bool ignoreErrors = false) : 
    DirIterator(path, false), ignoreErrors(ignoreErrors)
  { }

  explicit RecursiveDirIterator(const std::string& path, 
        const std::function<bool(std::string)>& filter, bool ignoreErrors = false) : 
    DirIterator(path, filter, false), ignoreErrors(ignoreErrors)
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
