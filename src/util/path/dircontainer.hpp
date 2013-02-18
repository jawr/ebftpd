#ifndef __UTIL_DIRCONTAINER_HPP
#define __UTIL_DIRCONTAINER_HPP

#include <utility>

namespace util { namespace path
{

template <typename IteratorType>
class GenericDirContainer
{
	IteratorType it;
  IteratorType endIt;
  
public:  
  explicit GenericDirContainer(IteratorType begin) :
    it(begin)
  { }
  
  template <typename... Args>
  explicit GenericDirContainer(Args&&... args) : 
    it(std::forward<Args>(args)...)
  { }

  IteratorType begin()
  {
    it.Rewind();
    return it;
  }
  IteratorType end() { return endIt; }
};

class DirIterator;
typedef GenericDirContainer<path::DirIterator> DirContainer;

class RecursiveDirIterator;
typedef GenericDirContainer<path::RecursiveDirIterator> RecursiveDirContainer;

class GlobIterator;
typedef GenericDirContainer<path::GlobIterator> GlobDirContainer;

} /* path namespace */
} /* util namespace */

#endif
