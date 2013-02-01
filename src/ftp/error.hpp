#ifndef __FTP_ERROR_HPP
#define __FTP_ERROR_HPP

#include <string>
#include "util/error.hpp"

namespace ftp
{
 
class ProtocolError : public util::RuntimeError
{
public:
  ProtocolError() : std::runtime_error("FTP protocol violation") { }
  ProtocolError(const std::string& message) : 
    std::runtime_error("FTP protocol violation: " + message) { }
};
 
class TransferAborted : public util::RuntimeError
{
public:
  TransferAborted() : std::runtime_error("Transfer aborted") { }
};

class ControlError : public util::RuntimeError
{
  std::exception_ptr eptr;
  
public:
  ControlError(const std::exception_ptr& eptr) :
    std::runtime_error("Control error"), eptr(eptr) { }
    
  void Rethrow() const
  {
    std::rethrow_exception(eptr);
  }
};

class MinimumSpeedError : public util::RuntimeError
{
  int limit;
  double speed;
  
public:
  MinimumSpeedError(int limit, double speed) :
    std::runtime_error("Minimum speed error"),
    limit(limit), speed(speed) { }
    
  int Limit() const { return limit; }
  double Speed() const { return speed; }
};
 
} /* ftp namespace */

#endif
