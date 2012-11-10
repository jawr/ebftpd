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
#include "util/passwd.hpp"

namespace util { namespace passwd
{

std::string GenerateSalt(const unsigned int length)
{
  CryptoPP::AutoSeededRandomPool prng;
  CryptoPP::SecByteBlock salt(length);
  prng.GenerateBlock(salt, length);
  return std::string(std::begin(salt), std::end(salt));
}

std::string HashPassword(const std::string& password, const std::string& salt)
{
  static const unsigned iterations = 1000;
  
  CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf2;
  
  byte key[CryptoPP::SHA256::DIGESTSIZE];
  pbkdf2.DeriveKey(key, sizeof(key), 0, 
                   reinterpret_cast<const byte*>(password.c_str()), password.length(),
                   reinterpret_cast<const byte*>(salt.c_str()), salt.length(), iterations);
                   
  return std::string(reinterpret_cast<char*>(key), sizeof(key));
}

std::string HexEncode(const std::string& data)
{
  CryptoPP::HexEncoder encoder;
  std::string result;
  encoder.Attach(new CryptoPP::StringSink(result));
  encoder.Put(reinterpret_cast<const byte*>(data.c_str()), data.length());
  encoder.MessageEnd();
  return result;
}

std::string HexDecode(const std::string& data)
{
  CryptoPP::HexDecoder decoder;
  std::string result;
  decoder.Attach(new CryptoPP::StringSink(result));
  decoder.Put(reinterpret_cast<const byte*>(data.c_str()), data.length());
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
