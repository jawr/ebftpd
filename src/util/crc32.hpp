#ifndef __UTIL_CRC_HPP
#define __UTIL_CRC_HPP

#include <sstream>
#include <iomanip>
#include <string>
#include <cstdint>
#include <zlib.h>

namespace util
{

class CRC32
{
  uint32_t checksum;
  
public:
  CRC32() : checksum(crc32(0, nullptr, 0)) { }
  
  void Update(const void* bytes, unsigned len)
  {
    checksum = crc32(checksum, static_cast<const unsigned char*>(bytes), len);
  }
  
  uint32_t Checksum() const { return checksum; }
  
  std::string HexString() const
  {
    std::ostringstream os;
    os << std::hex << std::uppercase << checksum;
    return os.str();
  }
};

} /* util namespace */

#endif
