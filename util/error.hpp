#ifndef __UTIL_ERROR_HPP
#define __UTIL_ERROR_HPP

#include <string>
#include <stdexcept>
#include <cerrno>

namespace util
{

std::string ErrnoToMessage(int errno_);

class RuntimeError : private virtual std::runtime_error
{
public:
  RuntimeError() : std::runtime_error("Runtime error") { }
  RuntimeError(const char* message) : std::runtime_error(message) { }
  std::string Message() const { return what(); }
};

class SystemError : public virtual RuntimeError
{
protected:
  int errno_;

public:
  SystemError(int errno_);
  
  int Errno() const { return errno_; }
};

class Error
{
	bool success;
	int errno_;
  bool validErrno;
	std::string message;

  Error(bool success) : success(success), errno_(0), validErrno(false),
    message(success ? "Success" : "Failure") { }
  Error(const std::string& message) : success(false), 
    errno_(0), validErrno(false), message(message) { }
  Error(int errno_) : success(false), errno_(errno_),
    validErrno(true), message(ErrnoToMessage(errno_)) { }
  
public:

  operator bool() const { return success; }
  
  int Errno() const { return errno_; }
  bool ValidErrno() const { return validErrno; }
  const std::string& Message() const { return message; }

  static Error Success() { return Error(true); }
  static Error Failure(const std::string message = "Unknown error")
  { return Error(message); }
  static Error Failure(int errno_) { return Error(errno_); }
};

} /* util namespace */

#endif
