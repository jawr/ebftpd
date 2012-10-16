#ifndef __FS_DIRCONTAINER_HPP
#define __FS_DIRCONTAINER_HPP

#include "fs/diriterator.hpp"
#include "fs/path.hpp"

namespace ftp
{
class Client;
}

namespace fs
{

class DirContainer
{
	DirIterator it;
  DirIterator endIt;
  
public:
	explicit DirContainer(const fs::Path& path) : it(path) { }
  explicit DirContainer(const ftp::Client& client, const fs::Path& path) :
    it(client, path) { }
    
  DirIterator& begin() { return it.Rewind(); }
  DirIterator& end() { return endIt; }
};

} /* fs namespace */

#endif
