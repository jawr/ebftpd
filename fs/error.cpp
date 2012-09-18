#include <cstring>
#include "error.hpp"

namespace fs
{

Error Error::successError(true);
Error Error::failureError(false);

std::string ErrnoToMessage(int errno_)
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


#ifdef FS_ERROR_TEST

#include <cerrno>
#include <iostream>

int main()
{
  using namespace fs;
  
  Error e = Error::Success();
  
  std::cout << "success: " << (e ? "true" : "false" )  << " " << e.Message() << " " << e.Errno() << std::endl;
  e = Error::Failure(ENOENT);
  
  std::cout << "fail: " << (e ? "true" : "false" ) << " " << e.Message() << " " << e.Errno() << std::endl;
  
  e = Error::Failure();
  std::cout << "fail: " << (e ? "true" : "false" ) << " " << e.Message() << " " << e.Errno() << std::endl;
  
}

#endif
