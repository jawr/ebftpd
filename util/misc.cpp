
#include <sys/time.h>
#include "util/misc.hpp"

namespace util
{

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

