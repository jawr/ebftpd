//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __UTIL_ENUMSTRINGS_HPP
#define __UTIL_ENUMSTRINGS_HPP

#include <string>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <cstring>

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
