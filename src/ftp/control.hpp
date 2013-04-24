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

#ifndef __FTP_CONTROL_HPP
#define __FTP_CONTROL_HPP

#include <memory>
#include <string>
#include "ftp/writeable.hpp"
#include "ftp/replycodes.hpp"
#include "ftp/format.hpp"

namespace boost
{
namespace posix_time
{
class time_duration;
}
}

namespace util { namespace net
{
class TCPListener;
class TCPSocket;
class Endpoint;
}
}

namespace ftp
{

class ControlImpl;

class Control : public Writeable
{
  std::unique_ptr<ControlImpl> pimpl;
  util::net::TCPSocket* socket;

  Control& operator=(Control&&) = delete;
  Control& operator=(const Control&) = delete;
  Control(Control&&) = delete;
  Control(const Control&) = delete;
  
public:
  Control();
  ~Control();
  
  void Accept(util::net::TCPListener& listener);
 
  std::string NextCommand(const boost::posix_time::time_duration* timeout = nullptr);
  
  ::ftp::Format PartFormat;
  ::ftp::Format Format;
  
  void PartReply(ReplyCode code, const std::string& message);
  void Reply(ReplyCode code, const std::string& message);

  void SetSingleLineReplies(bool singleLineReplies);
  
  bool SingleLineReplies() const;
  
  void NegotiateTLS();
  
  void Write(const char* buffer, size_t len);
  
  const util::net::Endpoint& RemoteEndpoint() const;
 
  const util::net::Endpoint& LocalEndpoint() const;
  
  bool IsTLS() const;
  std::string TLSCipher() const;
  
  void Interrupt();
  
  long long BytesRead() const;
  long long BytesWrite() const;
  
  std::string WaitForIdnt();
  
  friend class Data;
};

} /* ftp namespace */

#endif
