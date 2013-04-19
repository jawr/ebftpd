#ifndef __FS_DIRITERATOR_HPP
#define __FS_DIRITERATOR_HPP

#include "fs/path.hpp"
#include "util/path/diriterator.hpp"

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

class DirIterator : public util::path::DirIterator
{
  //const acl::User* user;
  
public:
  DirIterator() = default;
  DirIterator(const acl::User& user, const VirtualPath& path);
};

typedef util::path::GenericDirContainer<DirIterator> DirContainer;

} /* fs namespace */

#endif
