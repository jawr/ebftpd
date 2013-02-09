#ifndef __UTIL_DIRCONTAINER_HPP
#define __UTIL_DIRCONTAINER_HPP

#include <utility>

namespace util { namespace path
{

template <typename GenericDirIterator = DirIterator>
class DirContainer
{
	GenericDirIterator it;
  GenericDirIterator endIt;
  
public:  
  explicit DirContainer(GenericDirIterator begin) :
    it(begin)
  { }
  
  template <typename... Args>
  explicit DirContainer(Args... args) : 
    it(std::forward<Args>(args)...)
  { }

  void Rewind() { it.Rewind(); }
    
  GenericDirIterator begin() { return it; }
  GenericDirIterator end() { return endIt; }
};

class RecursiveDirIterator;
typedef DirContainer<path::RecursiveDirIterator> RecursiveDirContainer;

} /* path namespace */
} /* util namespace */

#endif
