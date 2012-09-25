#ifndef __UTIL_NET_ERROR_HPP
#define __UTIL_NET_ERROR_HPP

#include "util/error.hpp"

namespace util { namespace net
{

class NetworkError : public virtual util::RuntimeError
{
public:
  NetworkError() : std::runtime_error("Network error") { }
  NetworkError(const char* message) : std::runtime_error(message) { }
};

class NetworkSystemError : public util::SystemError, public virtual NetworkError
{
public:
  NetworkSystemError(int errno_);
};

class EndOfStream : public NetworkError
{
public:
  EndOfStream() : std::runtime_error("End of stream") { }
};

class BufferSizeExceeded : public NetworkError
{
public:
  BufferSizeExceeded() : std::runtime_error("Buffer size exceeded") { }
};

} /* net namespace */
} /* util namespace */

#endif
