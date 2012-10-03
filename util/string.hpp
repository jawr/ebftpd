#ifndef __UTIL_STRING_HPP
#define __UTIL_STRING_HPP
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
namespace util { namespace string
{
  bool BoolLexicalCast(const std::string& arg);
}
}
#endif
