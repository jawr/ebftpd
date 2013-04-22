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

#include <vector>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include "util/passwd.hpp"
#include "util/verify.hpp"

namespace util { namespace passwd
{

const unsigned int defaultSaltLength = 32;

std::string GenerateSalt(const unsigned int length)
{
  std::vector<unsigned char> salt;
  salt.resize(length);
  verify(RAND_bytes(salt.data(), length) == 1);
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
  os << std::hex << std::setfill('0') << std::uppercase;
  for (unsigned char ch : s)
  {
    os << std::setw(2) << static_cast<unsigned>(ch);
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

} /* passwd namespace */
} /* util namespace */

