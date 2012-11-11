#ifndef __FTP_DATA_HPP
#define __FTP_DATA_HPP

#include <sys/types.h>
#include "util/net/tcplistener.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/net/endpoint.hpp"
#include "ftp/readwriteable.hpp"
#include "ftp/transferstate.hpp"

namespace acl
{
class User;
}

namespace ftp
{

class Client;

enum class EPSVMode
{
  Normal,
  Full
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

class Data : public ReadWriteable
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

public:
  explicit Data(Client& client) :
    client(client),
    protection(false),
    pasvType(PassiveType::None),
    epsvMode(::ftp::EPSVMode::Normal),
    dataType(::ftp::DataType::Binary),
    sscnMode(::ftp::SSCNMode::Server),
    restartOffset(0),
    bytesRead(0),
    bytesWrite(0)
  {
  }

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
    if (state.Type() == TransferType::List)
      bytesWrite += state.Bytes();
  }
  
  size_t Read(char* buffer, size_t size)
  {
    return socket.Read(buffer, size);
  }
  
  void Write(const char* buffer, size_t len)
  {
    socket.Write(buffer, len);
  }
  
  TransferState& State() { return state; }
  const TransferState& State() const { return state; }
  
  void Interrupt() { socket.Shutdown(); }
  
  long long BytesRead() const { return bytesRead; }
  long long BytesWrite() const { return bytesWrite; }
};

} /* ftp namespace */

#endif
