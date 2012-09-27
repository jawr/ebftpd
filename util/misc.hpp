#ifndef __UTIL_MISC_HPP
#define __UTIL_MISC_HPP

#include <string>
#include <cstdlib>
#include <sstream>
#include <vector>

namespace util
{

std::string RandomString(size_t len, const std::string& validChars = 
                         "abcdefghijklmnopqrstuvwxyz0123456789");

template<class Collection>
void SplitToType(const std::string& s, char delim, Collection& result)
{
  std::istringstream is(s);
  typename Collection::value_type value;
  while (is >> value)
  {
    result.push_back(value);
    if (is.peek() == delim)
      is.ignore();
  }
}
                         
} /* util namespace */

#endif
