#if defined(CRYPTOPP)
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/secblock.h>
#else
#include <crypto++/osrng.h>
#include <crypto++/sha.h>
#include <crypto++/hex.h>
#include <crypto++/pwdbased.h>
#include <crypto++/secblock.h>
#endif

#include <sstream>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/x509v3.h>
#include "util/passwd.hpp"
#include "util/verify.hpp"

#include <cstdio>

namespace util { namespace passwd
{

std::string GenerateSalt(const unsigned int length)
{
/*  CryptoPP::AutoSeededRandomPool prng;
  CryptoPP::SecByteBlock salt(length);
  prng.GenerateBlock(salt, length);
  return std::string(std::begin(salt), std::end(salt));*/
  
  std::vector<unsigned char> salt;
  salt.reserve(length);
  verify(RAND_bytes(salt.data(), length));
  return std::string(salt.begin(), salt.end());
}

std::string HashPassword(const std::string& password, const std::string& salt)
{
  static const int iterations = 1000;
  
  std::vector<unsigned char> usalt(salt.begin(), salt.end());
  unsigned char key[32];
  
  verify(PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), 
                           usalt.data(), usalt.size(), iterations, 
                           EVP_sha256(), sizeof(key), key));
        
  return std::string(std::begin(key), std::end(key));
}

std::string HexEncode(const std::string& s)
{
  std::ostringstream os;
  os << std::hex << std::setfill('0');
  for (unsigned ch : s) 
  {
    os << std::setw(2) << ch;
  }
  return os.str();
}

std::string HexDecode(const std::string& s)
{
  std::ostringstream os;
  std::istringstream is1(s);
  std::string part;
  part.resize(2);
  while (is1.read(&part.front(), 2))
  {
    unsigned ch;
    std::istringstream is2(part);
    is2 >> std::hex >> ch;
    os << char(ch);
  }
  return os.str();
}

} /* crypto namespace */
} /* util namespace */


#ifdef UTIL_PASSWD_TEST

#include <iostream>
#include <cassert>

int main()
{
  {
    std::string message = "test message";
    std::string enc = util::passwd::HexEncode(message);
    std::string dec = util::passwd::HexDecode(enc);
    
    std::cout << "message: " << message << " " << message.length() << std::endl;
    std::cout << "encode: " << enc << " " << enc.length() << std::endl;
    std::cout << "decode: " << dec << " " << dec.length() << std::endl;
  }

  std::string salt = util::passwd::GenerateSalt();
  std::string password = "testpassword1234";
  
  std::cout << util::passwd::HexEncode(salt) << std::endl;
  
  std::string hashed = util::passwd::HashPassword(password, salt);
  
  std::cout << util::passwd::HexEncode(hashed) << std::endl;

  hashed = util::passwd::HashPassword(password, salt);
  std::cout << util::passwd::HexEncode(hashed) << std::endl;
  
  hashed = util::passwd::HashPassword(password, salt);
  std::cout << util::passwd::HexEncode(hashed) << std::endl;

  hashed = util::passwd::HashPassword("two", salt);
  std::cout << util::passwd::HexEncode(hashed) << std::endl;

  hashed = util::passwd::HashPassword("my mother fucking password", salt);
  std::cout << util::passwd::HexEncode(hashed) << std::endl;
  
  hashed = util::passwd::HashPassword(password, salt);
  std::cout << util::passwd::HexEncode(hashed) << std::endl;
}

#endif
