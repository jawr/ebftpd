#include "util/net/tlserror.hpp"

namespace util { namespace net
{

boost::thread_specific_ptr<TLSProtocolError::Buffer> TLSProtocolError::buffer;

char* TLSProtocolError::GetErrorString(unsigned long error)
{
  if (!buffer.get()) buffer.reset(new Buffer());
  return ERR_error_string(error, buffer.get()->buffer);
}

TLSProtocolError::TLSProtocolError(unsigned long error) :
  std::runtime_error(GetErrorString(error)),
  error(error)
{
}

TLSSystemError::TLSSystemError(int errno_) :
  std::runtime_error(ErrnoToMessage(errno_)),
  NetworkSystemError(errno_)
{
}

} /* net namespace */
} /* util namespace */
