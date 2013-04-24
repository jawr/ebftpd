//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
