#ifndef __UTIL_STRING_HPP
#define __UTIL_STRING_HPP

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace util { namespace string
{

bool BoolLexicalCast(std::string arg);

bool WildcardMatch(const std::string& pattern,
                   const std::string& str, bool iCase = false);

} /* string namespace */
} /* util namespace */

#endif
