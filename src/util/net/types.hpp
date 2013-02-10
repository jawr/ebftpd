#ifndef __UTIL_NET_TYPES_HPP
#define __UTIL_NET_TYPES_HPP

#include <sys/socket.h>

namespace util { namespace net
{

enum class SocketType : int
{
  Stream = SOCK_STREAM,
  Datagram = SOCK_DGRAM
};

} /* net namespace */
} /* util namespace */

#endif
