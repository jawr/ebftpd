#ifndef __FTP_CLIENT_HPP
#define __FTP_CLIENT_HPP

#include <string>
#include <cstdint>
#include <atomic>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "acl/user.hpp"
#include "acl/userprofile.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/net/tcplistener.hpp"
#include "util/thread.hpp"
#include "util/error.hpp"
#include "fs/path.hpp"
#include "cmd/dirlist.hpp"
#include "ftp/replycodes.hpp"
#include "ftp/data.hpp"
#include "ftp/control.hpp"
#include "ftp/xdupe.hpp"
#include "util/processreader.hpp"

namespace ftp 
{

enum class ClientState
{
  LoggedOut,
  WaitingPassword,
  LoggedIn,
  Finished,
  NotBeforeAuth,
  AnyState
};

class Client : public util::Thread
{
  mutable boost::mutex mutex;
  
  ::ftp::Control control;
  ::ftp::Data data;
  util::ProcessReader child;
  
  acl::User user;
  ::ftp::ClientState state;
  int passwordAttemps;
  fs::VirtualPath renameFrom;
  std::string ident;
  xdupe::Mode xdupeMode;
  std::string confirmCommand;
  std::string currentCommand;
  bool kickLogin;

  acl::UserProfile profile;

  boost::posix_time::ptime loggedInAt;
  boost::posix_time::ptime idleExpires;
  boost::posix_time::seconds idleTimeout;
  boost::posix_time::ptime idleTime;

  static std::atomic_bool siteopOnly;
  
  static const int maxPasswordAttemps = 3;
  
  void DisplayBanner();
  void ExecuteCommand(const std::string& commandLine);
  void Handle();
  bool CheckState(ClientState reqdState);
  void InnerRun();
  void Run();
  bool PreCheckAddress();
  void LookupIdent();
  void IdleReset(std::string commandLine)  ;
  
public:
  Client();
  ~Client();
     
  const acl::User& User() const { return user; }
  const acl::UserProfile& UserProfile() const { return profile; }
  
  bool Accept(util::net::TCPListener& server);
  bool IsFinished() const;
  void SetLoggedIn(const acl::UserProfile& profile, bool kicked);
  void SetWaitingPassword(const acl::User& user, bool kickLogin);
  bool VerifyPassword(const std::string& password);
  bool PasswordAttemptsExceeded() const;
  void SetRenameFrom(const fs::VirtualPath& path) { this->renameFrom = path; }
  const fs::VirtualPath& RenameFrom() const { return renameFrom; }
  
  bool KickLogin() const { return kickLogin; }
  
  ::ftp::Control& Control() { return control; }
  ::ftp::Data& Data() { return data; }
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
    boost::lock_guard<boost::mutex> lock(mutex);
    return currentCommand; 
  }
  
  const std::string& Ident() const
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    return ident;
  }
  
  const std::string& Address() const
  { return control.RemoteEndpoint().IP().ToString(); }
  
  ClientState State() const
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    return state;
  }

  void SetState(ClientState state);
  
  void Interrupt();
  
  void LogTraffic() const;
  
  const acl::UserProfile& Profile() const { return profile; }
  
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
};

} /* ftp namespace */

#endif
