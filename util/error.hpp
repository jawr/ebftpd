#ifndef __UTIL_ERROR_HPP
#define __UTIL_ERROR_HPP

#include <string>
#include <stdexcept>
#include <cerrno>

namespace util
{

std::string ErrnoToMessage(int errno_);

class SystemError : public std::runtime_error
{
  int errno_;
  bool validErrno;

public:
  SystemError() : 
    std::runtime_error("Unknown filesystem error"),
    errno_(-1), validErrno(false) { }
    
  SystemError(const std::string& message) : 
    std::runtime_error(message),
    errno_(-1), validErrno(false) { }
    
  SystemError(int errno_);
  
  int Errno() const { return errno_; }
  bool ValidErrno() const { return validErrno; }
};

class Error
{
	bool success;
	int errno_;
  bool validErrno;
	std::string message;

  Error(bool success) : success(success), errno_(0), validErrno(false),
    message(success ? "Success" : "Failure") { }
  Error(int errno_) : success(false), errno_(errno_),
    validErrno(true), message(ErrnoToMessage(errno_)) { }
  
  static Error successError;
  static Error failureError;
  
public:

  operator bool() const { return success; }
  
  int Errno() const { return errno_; }
  bool ValidErrno() const { return validErrno; }
  const std::string& Message() const { return message; }

  static Error Success() { return successError; }
  static Error Failure() { return failureError; }
  static Error Failure(int errno_) { return Error(errno_); }
};

} /* util namespace */

#endif
