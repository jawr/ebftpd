#include <cstring>
#include "util/error.hpp"

namespace util
{

std::string ErrnoToMessage(int errno_)
{
  char buffer[256];
  return strerror_r(errno_, buffer, sizeof(buffer));
}

SystemError::SystemError(int errno_) :
  std::runtime_error(ErrnoToMessage(errno_)),
  errno_(errno_)
{
}

} /* fs namespace */


#ifdef UTIL_ERROR_TEST

#include <cerrno>
#include <iostream>

int main()
{
  using namespace util;
  
  Error e = Error::Success();
  
  std::cout << "success: " << (e ? "true" : "false" )  << " " << e.Message() << " " << e.Errno() << std::endl;
  e = Error::Failure(ENOENT);
  
  std::cout << "fail: " << (e ? "true" : "false" ) << " " << e.Message() << " " << e.Errno() << std::endl;
  
  e = Error::Failure();
  std::cout << "fail: " << (e ? "true" : "false" ) << " " << e.Message() << " " << e.Errno() << std::endl;
  
}

#endif
