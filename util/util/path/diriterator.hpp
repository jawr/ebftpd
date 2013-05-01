#ifndef __UTIL_PATH_DIRITERATOR_HPP
#define __UTIL_PATH_DIRITERATOR_HPP

#include <iterator>
#include <memory>
#include <functional>
#include "util/path/diriteratorbase.hpp"

namespace boost { namespace filesystem3
{
class directory_iterator;
}
}

namespace util { namespace path
{

class DirIterator : public DirIteratorBase
{
public:
  enum ValueType { AbsolutePath, BasenameOnly };
  
private:
  std::string path;
  mutable std::string current;
  std::function<bool(const std::string&)> filter;
  std::shared_ptr<boost::filesystem3::directory_iterator> iter;
  ValueType valueType;

  void OpenDirectory();
  std::string& Current() const;
  
public:
  DirIterator();
  DirIterator(const std::string& path, ValueType valueType = BasenameOnly);
  DirIterator(const std::string& path, const std::function<bool(const std::string&)>& filter, 
              ValueType valueType = BasenameOnly);
  ~DirIterator();
  DirIterator& Rewind();
  bool operator==(const DirIteratorBase& rhs);
  bool operator!=(const DirIteratorBase& rhs);
  DirIterator& operator++();
  const std::string& operator*() const;
  const std::string* operator->() const;
};

} /* path namespace */
} /* util namespace */

#endif
