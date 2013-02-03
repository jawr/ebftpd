#ifndef __UTIL_SLICEBY8_HPP
#define __UTIL_SLICEBY8_HPP

#include <cstdint>
#include <sys/types.h>

namespace util { namespace sliceby8
{

uint32_t crc32(const void* data, size_t length, uint32_t previousCrc32 = 0);

} /* sliceby8 namespace */
} /* util namespace */

#endif
