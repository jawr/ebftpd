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

#ifndef __UTIL_NET_TLSERROR_HPP
#define __UTIL_NET_TLSERROR_HPP

#include <openssl/err.h>
#include <boost/thread/tss.hpp>
#include "util/net/error.hpp"

namespace util { namespace net
{

class TLSError : public virtual NetworkError
{
public:
  TLSError() : std::runtime_error("Unknown TLS error") { }
  TLSError(const std::string& message) :
    std::runtime_error(message) { }
};

class TLSProtocolError : public TLSError
{
  struct Buffer
  {
    char buffer[120];
  };

  unsigned long error;
  
  static boost::thread_specific_ptr<Buffer> buffer;
  static char* GetErrorString(unsigned long error);

public:
  TLSProtocolError(unsigned long code = ERR_get_error());
  
  int Error() const { return error; }
};

class TLSSystemError : public TLSError, public NetworkSystemError
{
public:
  TLSSystemError(int errno_);
};

} /* net namespace */
} /* util namespace */

#endif
