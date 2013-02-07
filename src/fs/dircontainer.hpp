#ifndef __FS_DIRCONTAINER_HPP
#define __FS_DIRCONTAINER_HPP

#include "fs/diriterator.hpp"

namespace acl
{
class User;
}

namespace fs
{

class Path;
class VirtualPath;

class DirContainer
{
	DirIterator it;
  DirIterator endIt;
  
public:
  explicit DirContainer(const acl::User& user, const VirtualPath& path) :
    it(user, path) { }
    
  void Rewind() { it.Rewind(); }
    
  DirIterator begin() { return it; }
  DirIterator end() { return endIt; }
};

} /* fs namespace */

#endif
