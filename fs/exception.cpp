#include <cstring>
#include "exception.hpp"

#include <iostream>

namespace fs
{

std::string FileSystemError::ErrnoToMessage(int errno_)
{
  char buffer[256];
  return strerror_r(errno_, buffer, sizeof(buffer));
}

FileSystemError::FileSystemError(int errno_) :
  std::runtime_error(ErrnoToMessage(errno_)),
  errno_(errno_),
  validErrno(true)
{
}

} /* fs namespace */
