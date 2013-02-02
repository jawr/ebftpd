#ifndef __UTIL_STRING_HPP
#define __UTIL_STRING_HPP

#include <boost/lexical_cast.hpp>
#include "util/error.hpp"

namespace util { namespace string
{

bool BoolLexicalCast(std::string arg);

bool WildcardMatch(const std::string& pattern,
                   const std::string& str, bool iCase = false);

std::string::size_type 
FindNthChar(const std::string& str, char ch, unsigned n);

std::string::size_type
FindNthNonConsecutiveChar(const std::string& str, 
    char ch, unsigned n);

std::string& CompressWhitespace(std::string& str);
std::string CompressWhitespaceCopy(const std::string& str);

std::string WordWrap(std::string& source, std::string::size_type length);

void TitleSimple(std::string& s);
std::string TitleSimpleCopy(const std::string& s);

inline bool IsASCIIOnly(const std::string& s)
{
  for (char ch : s)
  {
    unsigned char uCh = static_cast<unsigned char>(ch);
    if (uCh > 127) return false;
  }
  return true;
}

} /* string namespace */
} /* util namespace */

#endif
