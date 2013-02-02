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
