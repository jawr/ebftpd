

#ifdef UTIL_ENUM_TEST
#include "util/enum.hpp"
#include <iostream>
#include "util/test.hpp"

/*enum class Test { One, Two };

template <> const char* util::EnumStrings2<Test>::values[] = { "ONE", "TWO" };
*/
int main()
{
  using namespace util;
  std::cout << EnumStrings2<Test>::To(Test::Two) << std::endl;
  
  Test t = EnumStrings2<Test>::From("TWO");
  
  std::cout << EnumStrings2<Test>::To(t) << std::endl;
}

#endif