#include "util/net/error.hpp"

namespace util { namespace net
{

NetworkSystemError::NetworkSystemError(int errno_) :
  std::runtime_error(ErrnoToMessage(errno_)),
  SystemError(errno_)
{
}

} /* net namespace */
} /* util namespace */
