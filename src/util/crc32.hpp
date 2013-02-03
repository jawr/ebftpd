#ifndef __UTIL_CRC_HPP
#define __UTIL_CRC_HPP

#include <sstream>
#include <iomanip>
#include <string>
#include <cstdint>
#include "util/sliceby8.hpp"

namespace util
{

class CRC32
{
  uint32_t checksum;
  
public:
  CRC32() : checksum(0) { }
  virtual ~CRC32() { }
  
  virtual void Update(const uint8_t* bytes, unsigned len)
  {
    checksum = sliceby8::crc32(bytes, len, checksum);
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
