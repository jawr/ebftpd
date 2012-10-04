#include <sys/time.h>
#include "util/misc.hpp"

namespace util
{

std::string RandomString(size_t len, const std::string& validChars)
{

  struct timeval tv;
  gettimeofday(&tv,NULL);
  srand(tv.tv_usec * tv.tv_sec);

  std::string s;
  s.reserve(len);
  for (size_t i = 0; i < len; ++i)
    s += validChars[rand() % validChars.length()];

  return s;
}

} /* misc namespace */

#ifdef UTIL_MISC_TEST

#include <iostream>
#include <iterator>

int main()
{
  using namespace util;
  
  std::string s = "1,2,3,4";
  std::vector<int> fields;
  SplitToIntegral(s, ',', fields);
  
  std::copy(fields.begin(), fields.end(), std::ostream_iterator<int>(std::cout, "\n"));
}

#endif

