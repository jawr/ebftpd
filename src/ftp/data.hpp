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

  inline void SetProtection(bool protection) { this->protection = protection; }

  inline ::ftp::EPSVMode EPSVMode() const { return epsvMode; }
  inline void SetEPSVMode(::ftp::EPSVMode epsvMode) { this->epsvMode = epsvMode; }

  inline ::ftp::DataType DataType() const { return dataType; }
  inline void SetDataType(::ftp::DataType dataType) { this->dataType = dataType; }
  
  void InitPassive(util::net::Endpoint& ep, PassiveType pasvType);
  void InitActive(const util::net::Endpoint& ep);
  void Open(TransferType transferType);
  inline void Close()
  {
    socket.Close();
    state.Stop();
  }
  
  inline size_t Read(char* buffer, size_t size)
  { return socket.Read(buffer, size); }
  
  inline void Write(const char* buffer, size_t len)
  { socket.Write(buffer, len); }
  
  TransferState& State() { return state; }
  const TransferState& State() const { return state; }
};

} /* ftp namespace */

#endif
