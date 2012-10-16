#ifndef __UTIL_STRING_HPP
#define __UTIL_STRING_HPP

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

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
    
} /* string namespace */
} /* util namespace */

#endif
