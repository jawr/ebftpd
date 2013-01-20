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

std::string HexEncode(const std::string& data)
{
  //std::vector<unsigned char> udata(data.begin(), data.end());
  
  std::ostringstream encoded;
  for (unsigned char ch : data)
  {
    encoded << std::hex << std::setfill('0') << std::setw(2) << ch;
  }
  
  std::cout << encoded.str() << std::endl;
  
  return encoded.str();
/*  long len = data.length();
  std::shared_ptr<unsigned char> encoded(string_to_hex(data.c_str(), &len), &free);
  if (!encoded.get())
  {
    ERR_print_errors_fp(stdout);
    abort();
  }
  std::cout << std::string(&data[0], &data[len]) << std::endl;
  return std::string(&data[0], &data[len]);*/
/*  
  try
  {
  }
  catch (...)
  {
    free(encoded);
    throw;
  }
  CryptoPP::HexEncoder encoder;
  std::string result;
  encoder.Attach(new CryptoPP::StringSink(result));
  encoder.Put(reinterpret_cast<const byte*>(data.c_str()), data.length());
  encoder.MessageEnd();
  return result;*/
}

std::string HexDecode(const std::string& data)
{
  std::vector<unsigned char> udata(data.begin(), data.end());
  std::shared_ptr<char> decoded(hex_to_string(udata.data(), udata.size()));
  verify(decoded.get());
  return std::string(decoded.get());
/*  CryptoPP::HexDecoder decoder;
  std::string result;
  decoder.Attach(new CryptoPP::StringSink(result));
  decoder.Put(reinterpret_cast<const byte*>(data.c_str()), data.length());
  decoder.MessageEnd();
  return result;*/
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
