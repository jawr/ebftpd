#ifndef __UTIL_NET_TLSERROR_HPP
#define __UTIL_NET_TLSERROR_HPP

#include <openssl/err.h>
#include <boost/thread/tss.hpp>
#include "util/net/error.hpp"

namespace util { namespace net
{

class TLSError : public virtual NetworkError
{
public:
  TLSError() : std::runtime_error("Unknown TLS error") { }
  TLSError(const std::string& message) :
    std::runtime_error(message) { }
};

class TLSProtocolError : public TLSError
{
  unsigned long error;
  
  static boost::thread_specific_ptr<char> buffer;
  static char* GetErrorString(unsigned long error);

public:
  TLSProtocolError(unsigned long code = ERR_get_error());
  
  int Error() const { return error; }
};

class TLSSystemError : public TLSError, public NetworkSystemError
{
public:
  TLSSystemError(int errno_);
};

} /* net namespace */
} /* util namespace */

#endif
