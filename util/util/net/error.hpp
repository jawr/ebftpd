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

#ifndef __UTIL_NET_ERROR_HPP
#define __UTIL_NET_ERROR_HPP

#include "util/error.hpp"

namespace util { namespace net
{

class NetworkError : public virtual util::RuntimeError
{
public:
  NetworkError() : std::runtime_error("Network error") { }
  NetworkError(const std::string& message) : 
    std::runtime_error(message.c_str()) { }
};

class TimeoutError : public virtual NetworkError
{
public:
  TimeoutError() : std::runtime_error("Timeout error") { }
};

class NetworkSystemError : public util::SystemError, public virtual NetworkError
{
public:
  NetworkSystemError(int errno_);
};

class EndOfStream : public NetworkError
{
public:
  EndOfStream() : std::runtime_error("End of stream") { }
};

class BufferSizeExceeded : public NetworkError
{
public:
  BufferSizeExceeded() : std::runtime_error("Buffer size exceeded") { }
};

} /* net namespace */
} /* util namespace */

#endif
