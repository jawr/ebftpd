#include <crypto++/osrng.h>
#include <crypto++/sha.h>
#include <crypto++/hex.h>
#include "passwd.hpp"

using CryptoPP::AutoSeededRandomPool;
using CryptoPP::SHA256;
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;

namespace util { namespace passwd
{

std::string GenerateSalt(unsigned int length)
{
  AutoSeededRandomPool prng;
  byte salt[length];
  prng.GenerateBlock(salt, length);
  return std::string(&salt[0], &salt[length]);
}

std::string HashPassword(const std::string& password, const std::string& salt)
{
  size_t dataLength =password.length() + salt.length();
  byte data[dataLength];
  
  std::copy(password.begin(), password.end(), &data[0]);
  std::copy(salt.begin(), salt.end(), &data[password.length()]);
  
  byte digest[CryptoPP::SHA256::DIGESTSIZE];
  CryptoPP::SHA256 hash;
  hash.CalculateDigest(digest, data, dataLength);
  
  return std::string(&digest[0], &digest[sizeof(digest)]);
}

std::string HexEncode(const std::string& data)
{
  CryptoPP::HexEncoder encoder;
  std::string result;
  encoder.Attach(new CryptoPP::StringSink(result));
  encoder.Put((byte*) data.c_str(), data.length());
  encoder.MessageEnd();
  return result;
}

std::string HexDecode(const std::string& data)
{
  CryptoPP::HexEncoder decoder;
  std::string result;
  decoder.Attach(new CryptoPP::StringSink(result));
  decoder.Put((byte*) data.c_str(), data.length());
  decoder.MessageEnd();
  return result;
}

} /* crypto namespace */
} /* util namespace */


#ifdef UTIL_PASSWD_TEST

#include <iostream>
#include <cassert>

int main()
{
  std::string salt = util::passwd::GenerateSalt();
  std::string password = "testpassword1234";
  
  std::cout << util::passwd::HexEncode(salt) << std::endl;
  
  std::string hashed = util::passwd::HashPassword(password, salt);
  
  std::cout << util::passwd::HexEncode(hashed) << std::endl;

  hashed = util::passwd::HashPassword(password, salt);
  std::cout << util::passwd::HexEncode(hashed) << std::endl;
}

#endif
