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
