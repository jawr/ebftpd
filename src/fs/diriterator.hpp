#ifndef __FS_DIRITERATOR_HPP
#define __FS_DIRITERATOR_HPP

#include "fs/path.hpp"
#include "util/diriterator.hpp"

namespace acl
{
class User;
}

namespace fs
{

class DirIterator : public util::DirIterator
{
  const acl::User* user;
  
  util::Error Check(const fs::Path& path);
  
public:
  DirIterator() : user(nullptr) { }
  DirIterator(const acl::User& user, const VirtualPath& path) : 
    util::DirIterator(MakeReal(path).ToString()), user(&user) { }
};


} /* fs namespace */

#endif
