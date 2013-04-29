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

#ifndef __FTP_CLIENTIMPL_HPP
#define __FTP_CLIENTIMPL_HPP

#include <string>
#include <atomic>
#include <mutex>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include "acl/user.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/thread.hpp"
#include "fs/path.hpp"
#include "ftp/replycodes.hpp"
#include "ftp/data.hpp"
#include "ftp/control.hpp"
#include "ftp/xdupe.hpp"
#include "util/processreader.hpp"
#include "ftp/enums.hpp"

namespace util
{
namespace net
{
class TCPListener;
}
}


namespace ftp 
{

class Client;
enum class CounterResult;

class LoginGuard
{
  Client& client;
  bool loggedIn;
  boost::thread::id tid;
  
public:
  LoginGuard(Client& client) : 
    client(client), loggedIn(false)
  {
  }
  
  ~LoginGuard()
  {
    if (loggedIn) 
    {
      Logout();
    }
  }
  
  CounterResult Login(bool kicked, const boost::thread::id& tid);
  void Logout();
};


class Client;

class ClientImpl : public util::Thread
{
  mutable std::mutex mutex;
  
  Client& parent;
  ::ftp::Control control;
  ::ftp::Data data;
  util::ProcessReader child;
  
  LoginGuard loginGuard;
  std::atomic<bool> userUpdated;
  boost::optional<acl::User> user;
  ::ftp::ClientState state;
  int passwordAttemps;
  boost::optional<std::pair<fs::VirtualPath, std::string>> renameFrom;
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
  std::string SanitiseAddress(std::string address, LogAddresses log) const;
  
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
  void SetRenameFrom(const boost::optional<std::pair<fs::VirtualPath, std::string>>& from)
  { this->renameFrom = from; }
  const boost::optional<std::pair<fs::VirtualPath, std::string>>& RenameFrom() const
  { return renameFrom; }
  
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
  
  std::string IP(LogAddresses log = LogAddresses::NotLogging) const;
  std::string Ident(LogAddresses log = LogAddresses::NotLogging) const;
  std::string Hostname(LogAddresses log = LogAddresses::NotLogging) const;
  void HostnameLookup();
  std::string HostnameAndIP(LogAddresses log = LogAddresses::NotLogging) const;
  bool IdntUpdate(const std::string& ident, std::string ip,
                  const std::string& hostname);
  bool IdntParse(const std::string& command);
  
  void SetUserUpdated() { userUpdated = true; }  
};

} /* ftp namespace */

#endif
