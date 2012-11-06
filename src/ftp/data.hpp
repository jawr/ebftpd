#ifndef __FTP_DATA_HPP
#define __FTP_DATA_HPP

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
  LPSV
};

class Data : public ReadWriteable
{
  Client& client;
  util::net::TCPListener listener;
  util::net::TCPSocket socket;
  bool protection;
  bool passiveMode;
  util::net::Endpoint portEndpoint;
  ::ftp::EPSVMode epsvMode;
  ::ftp::DataType dataType;
  
  TransferState state;

public:
  explicit Data(Client& client) :
    client(client),
    protection(false),
    passiveMode(false),
    epsvMode(::ftp::EPSVMode::Normal),
    dataType(::ftp::DataType::ASCII)
  {
  }

  void SetProtection(bool protection) { this->protection = protection; }

  ::ftp::EPSVMode EPSVMode() const { return epsvMode; }
  void SetEPSVMode(::ftp::EPSVMode epsvMode) { this->epsvMode = epsvMode; }

  ::ftp::DataType DataType() const { return dataType; }
  void SetDataType(::ftp::DataType dataType) { this->dataType = dataType; }
  
  void InitPassive(util::net::Endpoint& ep, PassiveType pasvType);
  void InitActive(const util::net::Endpoint& ep);
  void Open(TransferType transferType);
  
  void Close()
  {
    socket.Close();
    state.Stop();
  }
  
  size_t Read(char* buffer, size_t size)
  { return socket.Read(buffer, size); }
  
  void Write(const char* buffer, size_t len)
  { socket.Write(buffer, len); }
  
  TransferState& State() { return state; }
  const TransferState& State() const { return state; }
  
  void Interrupt() { socket.Shutdown(); }
};

} /* ftp namespace */

#endif
