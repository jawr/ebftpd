#include "util/path/recursivediriterator.hpp"

namespace util { namespace path
{

std::string RecursiveDirIterator::NextEntry()
{
  if (subIt)
  {
    if (++(*subIt) != *subEnd)
    {
      return **subIt;
    }
    subIt = nullptr;
  }
  
  std::string entry = DirIterator::NextEntry();
  if (!entry.empty() && IsDirectory(entry))
  {
    try
    {
      subIt.reset(new RecursiveDirIterator(entry, filter, ignoreErrors));
      if (!subEnd) subEnd.reset(new RecursiveDirIterator());
    }
    catch (const util::SystemError&)
    {
      if (!ignoreErrors) throw;
    }
  }
  
  return entry;
}

} /* path namespace */
} /* util namespace */
