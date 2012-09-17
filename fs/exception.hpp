#ifndef __FS_EXCEPTION_HPP
#define __FS_EXCEPTION_HPP

#include <stdexcept>

namespace fs
{

class FileSystemError : public std::runtime_error
{
  int errno_;
  bool validErrno;
  
  std::string ErrnoToMessage(int errno_);
  
public:
  FileSystemError() : 
    std::runtime_error("Unknown filesystem error"),
    errno_(-1), validErrno(false) { }
    
  FileSystemError(const std::string& message) : 
    std::runtime_error(message),
    errno_(-1), validErrno(false) { }
    
  FileSystemError(int errno_);
  
  int Errno() const { return errno_; }
  bool ValidErrno() const { return validErrno; }
};


} /* ns namespace */

#endif
