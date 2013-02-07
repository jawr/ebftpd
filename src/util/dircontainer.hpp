#ifndef __UTIL_DIRCONTAINER_HPP
#define __UTIL_DIRCONTAINER_HPP

namespace util
{

class DirContainer
{
	DirIterator it;
  DirIterator endIt;
  
public:
  explicit DirContainer(const std::string& path) : it(path) { }
    
  void Rewind() { it.Rewind(); }
    
  DirIterator begin() { return it; }
  DirIterator end() { return endIt; }
};

} /* util namespace */

#endif
