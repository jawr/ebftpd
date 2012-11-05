#ifndef __FTP_CONTROL_HPP
#define __FTP_CONTROL_HPP

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "ftp/readwriteable.hpp"
#include "ftp/replycodes.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/pipe.hpp"

namespace util { namespace net
{
class TCPListener;
}
}

namespace ftp
{

class Control : public ReadWriteable
{
  util::net::TCPSocket socket;
  util::Pipe interruptPipe;
  ReplyCode lastCode;
  std::string commandLine;
  
  long bytesRead;
  long bytesWrite;
  
  void SendReply(ReplyCode code, bool part, const std::string& message);

  size_t Read(char* buffer, size_t size)
  { 
    size_t len = socket.Read(buffer, size);
    if (len > 0) bytesRead += len;
    return len;
  }
  
public:
  Control() : lastCode(CodeNotSet), bytesRead(0), bytesWrite(0) { }
  
  ~Control()
  {
    std::cout << "read: " << bytesRead << " " << "write: " << bytesWrite << std::endl;
  }
  
  void Accept(util::net::TCPListener& listener);
 
  std::string NextCommand(const boost::posix_time::time_duration& timeout);
  void PartReply(ReplyCode code, const std::string& message);
  void PartReply(const std::string& message);
  void Reply(ReplyCode code, const std::string& message);
  void Reply(const std::string& message);
  void MultiReply(ReplyCode code, const std::string& messages);

  void NegotiateTLS();
  
  void Write(const char* buffer, size_t len)
  {
    socket.Write(buffer, len);
    bytesWrite += len;
  }
  
  const util::net::Endpoint& RemoteEndpoint() const
  { return socket.RemoteEndpoint(); }
  
  const util::net::Endpoint& LocalEndpoint() const
  { return socket.LocalEndpoint(); }
  
  bool IsTLS() const { return socket.IsTLS(); }
  
  void Interrupt(){ socket.Shutdown(); }
  
  void LogTraffic();
};

} /* ftp namespace */

#endif
