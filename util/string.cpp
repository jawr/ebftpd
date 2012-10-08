#include <fnmatch.h>
#include "util/string.hpp"

namespace util { namespace string
{

bool BoolLexicalCast(std::string arg)
{
  boost::algorithm::to_lower(arg);
  if (arg == "yes") return true;
  else if (arg == "no") return false;
  return boost::lexical_cast<bool>(arg);
}

bool WildcardMatch(const std::string& pattern,
                   const std::string& str, bool iCase)
{
  return !fnmatch(pattern.c_str(), str.c_str(), iCase ? FNM_CASEFOLD : 0);
}

} /* string namespace */
} /* util namespace */

