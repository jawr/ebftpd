#ifndef __UTIL_STRING_HPP
#define __UTIL_STRING_HPP

#include <algorithm>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include "util/error.hpp"

namespace util
{

bool BoolLexicalCast(std::string arg);

bool WildcardMatch(const char* pattern,
                   const std::string& str, bool iCase = false);

inline bool WildcardMatch(const std::string& pattern,
                          const std::string& str, bool iCase = false)
{
  return WildcardMatch(pattern.c_str(), str, iCase);
}

template<typename Iterator>
inline bool WildcardMatch(Iterator begin, Iterator end, const std::string& str, bool iCase = false)
{
  return std::find_if(begin, end, [&](const std::string& pattern)
            {
              return WildcardMatch(pattern, str, iCase);
            }) != end;
}

template <typename Container>
inline bool WildcardMatch(const Container& container, const std::string& str, bool iCase = false)
{
  return WildcardMatch(std::begin(container), std::end(container), str, iCase);
}
                   
std::string::size_type 
FindNthChar(const std::string& str, char ch, unsigned n);

std::string::size_type
FindNthNonConsecutiveChar(const std::string& str, 
    char ch, unsigned n);

std::string& CompressWhitespace(std::string& str);
std::string CompressWhitespaceCopy(const std::string& str);

std::string WordWrap(std::string& source, std::string::size_type length);

void TitleSimple(std::string& s);
std::string TitleSimpleCopy(const std::string& s);

bool IsASCIIOnly(const std::string& s);

template <typename Iterator>
std::string Join(Iterator begin, Iterator end, const std::string& delim)
{
  std::string result;
  for (auto it = begin; it != end; ++it)
  {
    if (it != begin) result += delim;
    result += *it;
  }
  return result;
}

template <typename Container>
std::string Join(const Container& cont, const std::string& delim)
{
  return Join(cont.begin(), cont.end(), delim);
}

void Split(std::vector<std::string>& cont, const std::string& s, 
           const std::string& delims, bool tokenCompress = false);
           
void Trim(std::string& s);
std::string TrimCopy(const std::string& s);
void TrimRight(std::string& s);
std::string TrimRightCopy(const std::string& s);
void TrimLeft(std::string& s);
std::string TrimLeftCopy(const std::string& s);
void TrimRightIf(std::string& s, const std::string& chars);
std::string TrimRightCopyIf(const std::string& s, const std::string& chars);
void TrimLeftIf(std::string& s, const std::string& chars);
std::string TrimLeftCopyIf(const std::string& s, const std::string& chars);

void ToLower(std::string& s);
std::string ToLowerCopy(const std::string& s);

void ToUpper(std::string& s);
std::string ToUpperCopy(const std::string& s);

bool StartsWith(const std::string& s, const std::string& test);
bool EndsWith(const std::string& s, const std::string& test);

} /* util namespace */

#endif
