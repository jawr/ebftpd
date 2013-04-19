#ifndef __UTIL_RANDOMSTRING_HPP
#define __UTIL_RANDOMSTRING_HPP

#include <sys/types.h>
#include <string>
#include <random>

namespace util
{

class RandomString
{
  std::string str;

  char RandomChar(unsigned first, unsigned last);
  void GenerateValidChars(size_t length,
                          const std::string& validChars);

  void GenerateASCIIRange(size_t length,
                          unsigned firstASCII,
                          unsigned lastASCII);

public:
  static const unsigned firstPrintASCII = 32;
  static const unsigned lastPrintASCII = 126;

  static const std::string alphaNumeric;
  static const std::string alpha;
  static const std::string numeric;
  
  RandomString(size_t length, const std::string& validChars);
  RandomString(size_t length,
               unsigned firstASCII = firstPrintASCII, 
               unsigned lastASCII = lastPrintASCII);
               
  operator std::string() const { return str; }  
  const std::string& ToString() const { return str; }
};

} /* util namespace */

#endif
