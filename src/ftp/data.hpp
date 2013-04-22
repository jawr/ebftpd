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

#ifndef __FTP_DATA_HPP
#define __FTP_DATA_HPP

#include <memory>
#include <sys/types.h>
#include "util/net/tcplistener.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/net/endpoint.hpp"
#include "ftp/writeable.hpp"
#include "ftp/transferstate.hpp"
#include "util/enumstrings.hpp"

namespace acl
{
class User;
}

namespace ftp
{

class Client;

enum class EPSVMode
{
  Extended,
  Normal
};


enum class DataType
{
  ASCII,
  Binary
};

enum class PassiveType
{
  PASV,
  EPSV,
  LPSV,
  CPSV,
  None
};

enum class SSCNMode
{
  Server,
  Client
};

class Data : public Writeable
{
  Client& client;
  util::net::TCPListener listener;
  util::net::TCPSocket socket;
  bool protection;
  PassiveType pasvType;
  util::net::Endpoint portEndpoint;
  ::ftp::EPSVMode epsvMode;
  ::ftp::DataType dataType;
  ::ftp::SSCNMode sscnMode;
  off_t restartOffset;
  
  long long bytesRead;
  long long bytesWrite;
  
  TransferState state;
  
  void HandleControl(int revents);

public:
  explicit Data(Client& client);
  void SetProtection(bool protection) { this->protection = protection; }
  bool Protection() const { return protection; }

  ::ftp::EPSVMode EPSVMode() const { return epsvMode; }
  void SetEPSVMode(::ftp::EPSVMode epsvMode) { this->epsvMode = epsvMode; }

  ::ftp::SSCNMode SSCNMode() const { return sscnMode; }
  void SetSSCNMode(::ftp::SSCNMode sscnMode) { this->sscnMode = sscnMode; }
  
  ::ftp::DataType DataType() const { return dataType; }
  void SetDataType(::ftp::DataType dataType) { this->dataType = dataType; }
  
  void SetRestartOffset(off_t restartOffset) { this->restartOffset = restartOffset; }
  off_t RestartOffset() const { return restartOffset; }
  
  void InitPassive(util::net::Endpoint& ep, PassiveType pasvType);
  void InitActive(const util::net::Endpoint& ep);
  void Open(TransferType transferType);
  
  void Close()
  {
    restartOffset = 0;
    socket.Close();
    state.Stop();
  }
  
  size_t Read(char* buffer, size_t size);
  void Write(const char* buffer, size_t len);
  
  TransferState& State() { return state; }
  const TransferState& State() const { return state; }
  
  void Interrupt();
  
  long long BytesRead() const { return bytesRead; }
  long long BytesWrite() const { return bytesWrite; }
  
  bool IsFXP() const;
  
  bool ProtectionOkay() const;
};

} /* ftp namespace */

namespace util
{
template <> const char* util::EnumStrings<ftp::EPSVMode>::values[];
}

#endif
