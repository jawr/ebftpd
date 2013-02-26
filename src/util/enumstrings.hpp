#ifndef __UTIL_ENUMSTRINGS_HPP
#define __UTIL_ENUMSTRINGS_HPP

#include <string>
#include <algorithm>
#include <stdexcept>
#include <cassert>

namespace util
{

template <typename Enum>
struct EnumStrings
{
  static const char* values[];
};

template <typename Enum>
Enum EnumFromString(const std::string& s)
{
  for (size_t i = 0; EnumStrings<Enum>::values[i][0] != '\0'; ++i)
  {
    if (!strcasecmp(EnumStrings<Enum>::values[i], s.c_str()))
      return static_cast<Enum>(i);
  }
  throw std::out_of_range("Invalid enum string");
}

template <typename Enum>
bool EnumFromString(const std::string& s, Enum& e)
{
  try
  {
    e = EnumFromString<Enum>(s);
    return true;
  }
  catch (const std::out_of_range&)
  {
    return false;
  }
}


template <typename Enum>
std::string EnumToString(Enum e)
{
  int i = static_cast<int>(e);
  assert(i >= 0);
  return EnumStrings<Enum>::values[i];
}

} /* util namespace */

#endif
