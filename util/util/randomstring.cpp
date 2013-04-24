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

#include "util/randomstring.hpp"

namespace util
{

const std::string RandomString::alphaNumeric =
  "abcdefghijklmnopqrstuvwxyz"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "1234567890";
  
const std::string RandomString::alpha =
  "abcdefghijklmnopqrstuvwxyz"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  
const std::string RandomString::numeric = 
  "1234567890";
  
RandomString::RandomString(size_t length, const std::string& validChars)
{
  GenerateValidChars(length, validChars);
}

RandomString::RandomString(size_t length,
                           unsigned firstASCII, 
                           unsigned lastASCII)
{
  GenerateASCIIRange(length, firstASCII, lastASCII);
}

char RandomString::RandomChar(unsigned first, unsigned last)
{
  std::random_device rd;
  std::mt19937 engine(rd());
  std::uniform_int_distribution<char> dist(first, last);
  return dist(engine);
}

void RandomString::GenerateValidChars(size_t length,
      const std::string& validChars)
{
  str.reserve(length);
  while (str.length() < length)
    str += validChars[RandomChar(0, validChars.length() - 1)];
}

void RandomString::GenerateASCIIRange(size_t length,
      unsigned firstASCII, unsigned lastASCII)
{
  str.reserve(length);
  while (str.length() < length)
    str += RandomChar(firstASCII, lastASCII);
}

} /* util namespace */
