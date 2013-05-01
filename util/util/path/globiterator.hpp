#ifndef __UTIL_PATH_GLOBITERATOR_HPP
#define __UTIL_PATH_GLOBITERATOR_HPP

#include "util/path/diriteratorbase.hpp"

namespace boost { namespace filesystem3
{
class recursive_directory_iterator;
}
}

namespace util { namespace path
{

class GlobIterator : public DirIteratorBase
{
  std::vector<std::string> pathTokens;
  mutable std::string current;
  std::function<bool(const std::string&)> filter;
  std::shared_ptr<boost::filesystem3::recursive_directory_iterator> iter;
  bool recursive;
  bool trailingSlash;
  
  void TokenizePath(std::string pathMask);
  void OpenDirectory();
  void Next();
  
public:
  GlobIterator();
  GlobIterator(const std::string& pathMask, bool recursive = false);
  GlobIterator(const std::string& pathMask, const std::function<bool(const std::string&)>& filter, 
               bool recursive = false);
  ~GlobIterator();
  GlobIterator& Rewind();
  bool operator==(const DirIteratorBase& rhs);
  bool operator!=(const DirIteratorBase& rhs);
  GlobIterator& operator++();
  const std::string& operator*() const;
  const std::string* operator->() const;
};

} /* path namespace */
} /* util namespace */

#endif
