#ifndef __FTP_CLIENTIMPL_HPP
#define __FTP_CLIENTIMPL_HPP

#include <string>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include "acl/user.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/net/tcplistener.hpp"
#include "util/thread.hpp"
#include "util/error.hpp"
#include "fs/path.hpp"
#include "ftp/replycodes.hpp"
#include "ftp/data.hpp"
#include "ftp/control.hpp"
#include "ftp/xdupe.hpp"
#include "util/processreader.hpp"
#include "ftp/clientstate.hpp"

namespace ftp 
{

class Client;

class ClientImpl : public util::Thread
{
  mutable std::mutex mutex;
  
  Client& parent;
  ::ftp::Control control;
  ::ftp::Data data;
  util::ProcessReader child;
  
  std::atomic<bool> userUpdated;
  boost::optional<acl::User> user;
  ::ftp::ClientState state;
  int passwordAttemps;
  fs::VirtualPath renameFrom;
  xdupe::Mode xdupeMode;
  std::string confirmCommand;
  std::string currentCommand;
  bool kickLogin;

  boost::posix_time::ptime loggedInAt;
  boost::posix_time::ptime idleExpires;
  boost::posix_time::seconds idleTimeout;
  boost::posix_time::ptime idleTime;

  std::string ident;
  std::string ip;
  std::string hostname;
  
  static std::atomic_bool siteopOnly;
  
  static const int maxPasswordAttemps = 3;
  
  void DisplayBanner();
  void ExecuteCommand(const std::string& commandLine);
  void Handle();
  bool CheckState(ClientState reqdState);
  void InnerRun();
  void Run();
  void LookupIdent();
  void IdleReset(std::string commandLine)  ;
  bool ReloadUser();
  
  ClientImpl& operator=(ClientImpl&&) = delete;
  ClientImpl& operator=(const ClientImpl&) = delete;
  ClientImpl(ClientImpl&&) = delete;
  ClientImpl(const ClientImpl&) = delete;
  
public:
  ClientImpl(Client& parent);
  ~ClientImpl();
     
  acl::User& User() { return *user; }
  const acl::User& User() const { return *user; }
  
  bool Accept(util::net::TCPListener& server);
  bool IsFinished() const;
  void SetLoggedIn(bool kicked);
  void SetWaitingPassword(const acl::User& user, bool kickLogin);
  bool VerifyPassword(const std::string& password);
  bool PasswordAttemptsExceeded() const;
  void SetRenameFrom(const fs::VirtualPath& path) { this->renameFrom = path; }
  const fs::VirtualPath& RenameFrom() const { return renameFrom; }
  
  bool KickLogin() const { return kickLogin; }
  
  ::ftp::Control& Control() { return control; }
  const ::ftp::Control& Control() const { return control; }
  
  ::ftp::Data& Data() { return data; }
  const ::ftp::Data& Data() const { return data; }
  
  util::ProcessReader& Child() { return child; }

  void SetIdleTimeout(const boost::posix_time::seconds& idleTimeout)
  { this->idleTimeout = idleTimeout; }
  const boost::posix_time::seconds& IdleTimeout() const
  { return idleTimeout; }
  
  const boost::posix_time::ptime LoggedInAt() const
  { return loggedInAt; }
  
  void SetXDupeMode(xdupe::Mode xdupeMode)
  { this->xdupeMode = xdupeMode; }
  xdupe::Mode XDupeMode() const { return xdupeMode; }
  
  bool IsFxp(const util::net::Endpoint& ep) const;
  
  bool ConfirmCommand(const std::string& argStr);

  boost::posix_time::seconds IdleTime() const 
  { 
    namespace pt = boost::posix_time;
    pt::time_duration diff = pt::second_clock::local_time() - idleTime;
    return pt::seconds(diff.total_seconds());
  }
  
  const std::string& CurrentCommand() const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return currentCommand; 
  }
  
  ClientState State() const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return state;
  }

  void SetState(ClientState state);
  
  void Interrupt();
  
  void LogTraffic() const;
  
  static bool SetSiteopOnly()
  {
    bool expected = false;
    return siteopOnly.compare_exchange_strong(expected, true);
  }
  
  static bool SetReopen()
  {
    bool expected = true; 
    return siteopOnly.compare_exchange_strong(expected, false);
  }
  
  static bool IsSiteopOnly()
  { return siteopOnly; }
  
  bool PostCheckAddress();
  bool PreCheckAddress();
  
  std::string IP() const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return ip;
  }
  
  std::string Ident() const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return ident;
  }
  
  std::string Hostname() const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return hostname;
  }
  void HostnameLookup();
  std::string HostnameAndIP() const;
  bool IdntUpdate(const std::string& ident, std::string ip,
                  const std::string& hostname);
  bool IdntParse(const std::string& command);
  
  void SetUserUpdated() { userUpdated = true; }  
};

} /* ftp namespace */

#endif
