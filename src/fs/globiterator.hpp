#ifndef __FS_GLOBITERATOR_HPP
#define __FS_GLOBITERATOR_HPP

#include "fs/path.hpp"
#include "util/path/globiterator.hpp"

namespace acl
{
class User;
}

namespace util { namespace path
{
template <typename IteratorType> class GenericDirContainer;
}
}

namespace fs
{

class GlobIterator : public util::path::GlobIterator
{
  const acl::User* user;
  mutable std::string current;
  
  std::string& Current() const
  {
    if (current.empty()) current = MakeVirtual(fs::RealPath(**iter)).ToString();
    return current;
  }
  
public:
  GlobIterator() : user(nullptr) { }
  GlobIterator(const acl::User& user, const VirtualPath& path, Flags flags = NoFlags);
  
  GlobIterator& operator++()
  {
    current.clear();
    ++(*iter);
    return *this;
  }
  
  const std::string& operator*() const
  {
    return Current();
  }
  const std::string* operator->() const
  {
    return &Current();
  }
};

typedef util::path::GenericDirContainer<GlobIterator> GlobContainer;

} /* fs namespace */

#endif
