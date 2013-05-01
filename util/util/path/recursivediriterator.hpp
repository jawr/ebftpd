#ifndef __UTIL_PATH_RECURSIVEDIRITERATOR_HPP
#define __UTIL_PATH_RECURSIVEDIRITERATOR_HPP

#include <iterator>
#include <memory>
#include <functional>
#include "util/path/diriteratorbase.hpp"

namespace boost { namespace filesystem3
{
class recursive_directory_iterator;
}
}

namespace util { namespace path
{

class RecursiveDirIterator : public DirIteratorBase
{
  std::string path;
  mutable std::string current;
  std::function<bool(const std::string&)> filter;
  std::shared_ptr<boost::filesystem3::recursive_directory_iterator> iter;

  void OpenDirectory();
  
public:
  RecursiveDirIterator();
  RecursiveDirIterator(const std::string& path);
  RecursiveDirIterator(const std::string& path, const std::function<bool(const std::string&)>& filter);
  ~RecursiveDirIterator();
  RecursiveDirIterator& Rewind();
  bool operator==(const DirIteratorBase& rhs);
  bool operator!=(const DirIteratorBase& rhs);
  RecursiveDirIterator& operator++();
  const std::string& operator*() const;
  const std::string* operator->() const;
};

} /* path namespace */
} /* util namespace */

#endif
