#ifndef __UTIL_PATH_DIRITERATORBASE_HPP
#define __UTIL_PATH_DIRITERATORBASE_HPP

#include <iterator>

namespace util { namespace path
{

class DirIteratorBase :
  public std::iterator<std::forward_iterator_tag, std::string>
{
public:
  virtual ~DirIteratorBase() { }  
  virtual DirIteratorBase& Rewind() = 0;
  virtual bool operator==(const DirIteratorBase& rhs) = 0;  
  virtual bool operator!=(const DirIteratorBase& rhs) = 0;  
  virtual DirIteratorBase& operator++() = 0;
  virtual const std::string& operator*() const = 0;
  virtual const std::string* operator->() const = 0;
};

} /* path namespace */
} /* util namespace */

#endif
