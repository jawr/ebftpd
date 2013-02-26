#ifndef __UTIL_PASSWD_HPP
#define __UTIL_PASSWD_HPP

#include <string>

namespace util { namespace passwd
{

extern const unsigned int defaultSaltLength;

std::string GenerateSalt(unsigned int length = defaultSaltLength);
std::string HashPassword(const std::string& password, const std::string& salt);
std::string HexEncode(const std::string& data);
std::string HexDecode(const std::string& data);

} /* passwd namespace */
} /* util namespace */

#endif