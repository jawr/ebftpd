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
  
  virtual uint32_t Checksum() const { return checksum; }
  
  virtual std::string HexString() const
  {
    std::ostringstream os;
    os << std::hex << std::uppercase << checksum;
    return os.str();
  }
};

} /* util namespace */

#endif
