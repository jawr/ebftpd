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

#include <iomanip>

#include <cctype>
#include <sstream>
#include <fnmatch.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "util/string.hpp"

namespace util
{

bool BoolLexicalCast(std::string arg)
{
  boost::to_lower(arg);
  if (arg == "yes") return true;
  else if (arg == "no") return false;
  throw std::bad_cast();
}

bool WildcardMatch(const char* pattern, const std::string& str, bool iCase)
{
  return !fnmatch(pattern, str.c_str(), iCase ? FNM_CASEFOLD : 0);
}

std::string::size_type 
FindNthChar(const std::string& str, char ch, unsigned n)
{
  std::string::size_type pos = str.find_first_of(ch);
  while (--n && pos != std::string::npos)
    pos = str.find_first_of(ch, pos + 1);
  return pos;
}

std::string::size_type
FindNthNonConsecutiveChar(const std::string& str, 
    char ch, unsigned n)
{
  std::string::size_type pos = str.find_first_of(ch);
  while (--n && pos != std::string::npos)
  {
    while (str[pos] == ch && pos != std::string::npos)
      ++pos;
    if (pos == std::string::npos) break;
    pos = str.find_first_of(ch, pos + 1);
  }
  return pos;
}

std::string& CompressWhitespace(std::string& str)
{
  for (auto it = str.begin(); it != str.end();)
    if (std::isspace(*it++))
      while (it != str.end() && std::isspace(*it))
        it = str.erase(it);

  if (str.empty()) return str;
  if (std::isspace(*str.begin())) 
    str.erase(str.begin());
  
  if (str.empty()) return str;
  if (std::isspace(*(str.end() - 1)))
    str.erase(str.end() - 1);
    
  return str;
}

std::string CompressWhitespaceCopy(const std::string& str)
{
  std::string temp(str);
  return CompressWhitespace(temp);
}

std::string WordWrap(std::string& source, std::string::size_type length)
{
  if (source.length() < length)
  {
    std::string part = source;
    source.clear();
    return part;
  }
  
  std::string part = source.substr(0, length);
  std::string::size_type pos = part.find_last_of("- \t");
  if (pos == std::string::npos)
  { 
    source.erase(0, part.length());
    return part;
  }
  source.erase(0, pos + 1);
  return part.substr(0, pos);
}

void TitleSimple(std::string& s)
{
  bool newWord = true;
  for (char& ch : s)
  {
    if (!std::isspace(ch))
    {
      if (newWord)
      {
        ch = std::toupper(ch);
        newWord = false;
      }
    }
    else
      newWord = true;
  }
}

std::string TitleSimpleCopy(const std::string& s)
{
  std::string result(s);
  TitleSimple(result);
  return result;
}

bool IsASCIIOnly(const std::string& s)
{
  for (char ch : s)
  {
    unsigned char uCh = static_cast<unsigned char>(ch);
    if (uCh > 127) return false;
  }
  return true;
}

void Split(std::vector<std::string>& cont, const std::string& s, 
           const std::string& delims, bool tokenCompress)
{
  boost::split(cont, s, boost::is_any_of(delims), tokenCompress ? 
               boost::token_compress_on : boost::token_compress_off);
}

void Trim(std::string& s)
{
  boost::trim(s);
}

std::string TrimCopy(const std::string& s)
{
  std::string temp(s);
  Trim(temp);
  return temp;
}

void TrimRight(std::string& s)
{
  boost::trim_right(s);
}

std::string TrimRightCopy(const std::string& s)
{
  std::string temp(s);
  TrimRight(temp);
  return temp;
}

void TrimLeft(std::string& s)
{
  boost::trim_left(s);
}

std::string TrimLeftCopy(const std::string& s)
{
  std::string temp(s);
  TrimLeft(temp);
  return temp;
}

void TrimRightIf(std::string& s, const std::string& chars)
{
  boost::trim_right_if(s, boost::is_any_of(chars));
}

std::string TrimRightCopyIf(const std::string& s, const std::string& chars)
{
  std::string temp(s);
  TrimRightIf(temp, chars);
  return temp;
}

void TrimLeftIf(std::string& s, const std::string& chars)
{
  boost::trim_left_if(s, boost::is_any_of(chars));
}

std::string TrimLeftCopyIf(const std::string& s, const std::string& chars)
{
  std::string temp(s);
  TrimLeftIf(temp, chars);
  return temp;
}

void ToLower(std::string& s)
{
  boost::to_lower(s);
}

std::string ToLowerCopy(const std::string& s)
{
  std::string temp(s);
  ToLower(temp);
  return temp;
}

void ToUpper(std::string& s)
{
  boost::to_upper(s);
}

std::string ToUpperCopy(const std::string& s)
{
  std::string temp(s);
  ToUpper(temp);
  return temp;
}

bool StartsWith(const std::string& s, const std::string& test)
{
  return boost::starts_with(s, test);
}

bool EndsWith(const std::string&s, const std::string& test)
{
  return boost::ends_with(s, test);
}

double StrToDouble(const std::string& s)
{
  try
  {
    size_t pos;
    double value = std::stod(s, &pos);
    if (pos != s.length()) throw std::bad_cast();
    return value;
  }
  catch (const std::logic_error&)
  {
    throw std::bad_cast();
  }
}

float StrToFloat(const std::string& s)
{
  try
  {
    size_t pos;
    float value = std::stof(s, &pos);
    if (pos != s.length()) throw std::bad_cast();
    return value;
  }
  catch (const std::logic_error&)
  {
    throw std::bad_cast();
  }
}

int StrToInt(const std::string& s)
{
  try
  {
    size_t pos;
    int value = std::stoi(s, &pos);
    if (pos != s.length()) throw std::bad_cast();
    return value;
  }
  catch (const std::logic_error&)
  {
    throw std::bad_cast();
  }
}

long StrToLong(const std::string& s)
{
  try
  {
    size_t pos;
    long value = std::stol(s, &pos);
    if (pos != s.length()) throw std::bad_cast();
    return value;
  }
  catch (const std::logic_error&)
  {
    throw std::bad_cast();
  }
}

long double StrToLDouble(const std::string& s)
{
  try
  {
    size_t pos;
    long double value = std::stold(s, &pos);
    if (pos != s.length()) throw std::bad_cast();
    return value;
  }
  catch (const std::logic_error&)
  {
    throw std::bad_cast();
  }
}

long long StrToLLong(const std::string& s)
{
  try
  {
    size_t pos;
    long long value = std::stoll(s, &pos);
    if (pos != s.length()) throw std::bad_cast();
    return value;
  }
  catch (const std::logic_error&)
  {
    throw std::bad_cast();
  }
}

unsigned long StrToULong(const std::string& s)
{
  try
  {
    size_t pos;
    unsigned long value = std::stoul(s, &pos);
    if (pos != s.length()) throw std::bad_cast();
    return value;
  }
  catch (const std::logic_error&)
  {
    throw std::bad_cast();
  }
}

unsigned long long StrToULLong(const std::string& s)
{
  try
  {
    size_t pos;
    unsigned long long value = std::stoull(s, &pos);
    if (pos != s.length()) throw std::bad_cast();
    return value;
  }
  catch (const std::logic_error&)
  {
    throw std::bad_cast();
  }
}

std::string ToString(long double v, int precision, bool fixed)
{
  std::ostringstream os;
  if (precision >= 0) os << std::setprecision(precision);
  if (fixed) os << std::fixed;
  os << v;
  return os.str();
}

} /* util namespace */
