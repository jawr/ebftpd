#include <cctype>
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

} /* string namespace */
} /* util namespace */

#ifdef UTIL_STRING_TEST

#include <iostream>

int main()
{
  using namespace util::string;

  std::string s = "    hello how   are you   today !";
  
  std::cout << CompressWhitespace(s) << std::endl;

  while (!s.empty())
    std::cout << WordWrap(s, 10) << std::endl;
  
}

#endif
