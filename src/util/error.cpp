#include <cstring>
#include "util/error.hpp"

namespace util
{

#if defined(__CYGWIN__ ) || \
  ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && \
   !defined(_GNU_SOURCE))
std::string ErrnoToMessage(int errno_)
{
  char buffer[256];
  if (strerror_r(errno_, buffer, sizeof(buffer)) < 0) return "";
  return buffer;
}
#else
std::string ErrnoToMessage(int errno_)
{
  char buffer[256], *p = strerror_r(errno_, buffer, sizeof(buffer));
  return p ? p : "";
}
#endif

SystemError::SystemError(int errno_) :
  std::runtime_error(ErrnoToMessage(errno_)),
  errno_(errno_)
{
}

} /* fs namespace */
