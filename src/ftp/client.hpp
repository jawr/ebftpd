#ifndef __FTP_CLIENT_HPP
#define __FTP_CLIENT_HPP

#include <memory>
#include <string>
#include "acl/types.hpp"
#include "ftp/enums.hpp"

namespace boost
{
namespace posix_time
{
class ptime;
class seconds;
}
}

namespace acl
{
class User;
}

namespace fs
{
class VirtualPath;
}

namespace util
{
class ProcessReader;
namespace net
{
class TCPListener;
class Endpoint;
}
}

namespace ftp 
{

namespace xdupe
{
enum class Mode : unsigned;
}

class ClientImpl;
class Control;
class Data;

class Client : public std::enable_shared_from_this<Client>
{
  std::unique_ptr<ClientImpl> pimpl;
  
public:
  Client();
  ~Client();
     
  acl::User& User();
  const acl::User& User() const;
  
  bool Accept(util::net::TCPListener& server);
  bool IsFinished() const;
  void SetLoggedIn(bool kicked);
  void SetWaitingPassword(const acl::User& user, bool kickLogin);
  bool VerifyPassword(const std::string& password);
  bool PasswordAttemptsExceeded() const;
  void SetRenameFrom(const fs::VirtualPath& path);
  const fs::VirtualPath& RenameFrom() const;
  
  bool KickLogin() const;
  
  ::ftp::Control& Control();
  const ::ftp::Control& Control() const;
  
  ::ftp::Data& Data();
  const ::ftp::Data& Data() const;
  
  util::ProcessReader& Child();

  void SetIdleTimeout(const boost::posix_time::seconds& idleTimeout);
  const boost::posix_time::seconds& IdleTimeout() const;
  
  const boost::posix_time::ptime LoggedInAt() const;
  void SetXDupeMode(xdupe::Mode xdupeMode);
  xdupe::Mode XDupeMode() const;
  
  bool IsFxp(const util::net::Endpoint& ep) const;
  
  bool ConfirmCommand(const std::string& argStr);

  boost::posix_time::seconds IdleTime() const;
  const std::string& CurrentCommand() const;
  ClientState State() const;
  void SetState(ClientState state);
  void Interrupt();
  void LogTraffic() const;
  static bool SetSiteopOnly();
  static bool SetReopen();
  static bool IsSiteopOnly();
  bool PostCheckAddress();
  bool PreCheckAddress();
  std::string IP(LogAddresses log = LogAddresses::NotLogging) const;  
  std::string Ident(LogAddresses log = LogAddresses::NotLogging) const;
  std::string Hostname(LogAddresses log = LogAddresses::NotLogging) const;
  void HostnameLookup();
  std::string HostnameAndIP(LogAddresses log = LogAddresses::NotLogging) const;
  bool IdntUpdate(const std::string& ident, std::string ip,
                  const std::string& hostname);
  bool IdntParse(const std::string& command);
  void SetUserUpdated();
  
  void Start();
  void Join();
  bool TryJoin();
};

} /* ftp namespace */

#endif
