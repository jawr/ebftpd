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

#include "util/net/tlserror.hpp"

namespace util { namespace net
{

boost::thread_specific_ptr<TLSProtocolError::Buffer> TLSProtocolError::buffer;

char* TLSProtocolError::GetErrorString(unsigned long error)
{
  if (!buffer.get()) buffer.reset(new Buffer());
  return ERR_error_string(error, buffer.get()->buffer);
}

TLSProtocolError::TLSProtocolError(unsigned long error) :
  std::runtime_error(GetErrorString(error)),
  error(error)
{
}

TLSSystemError::TLSSystemError(int errno_) :
  std::runtime_error(ErrnoToMessage(errno_)),
  NetworkSystemError(errno_)
{
}

} /* net namespace */
} /* util namespace */
