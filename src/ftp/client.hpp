#ifndef __FTP_CLIENT_HPP
#define __FTP_CLIENT_HPP

#include <string>
#include <cstdint>
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
  
  fs::Path workDir;
  acl::User user;
  ::ftp::ClientState state;
  int passwordAttemps;
  fs::Path renameFrom;
  std::string ident;
  ::ftp::XDupeMode xdupeMode;
  std::string confirmCommand;
  std::string currentCommand;

  acl::UserProfile profile;

  boost::posix_time::ptime loggedInAt;
  boost::posix_time::ptime idleExpires;
  boost::posix_time::seconds idleTimeout;
  boost::posix_time::ptime idleTime;
  
  static const int maxPasswordAttemps = 3;
  
  void DisplayBanner();
  void ExecuteCommand(const std::string& commandLine);
  void Handle();
  bool CheckState(ClientState reqdState);
  void Run();
  void LookupIdent();
  void IdleReset(const std::string& commandLine)  ;
  
public:
  Client();
  ~Client();
     
  const fs::Path& WorkDir() const { return workDir; }
  const acl::User& User() const { return user; }
  const acl::UserProfile& UserProfile() const { return profile; }
  
  bool Accept(util::net::TCPListener& server);
  bool IsFinished() const;
  void SetLoggedIn();
  void SetWaitingPassword(const acl::User& user);
  bool VerifyPassword(const std::string& password);
  bool PasswordAttemptsExceeded() const;
  void SetWorkDir(const fs::Path& workDir);
  void SetRenameFrom(const fs::Path& path) { this->renameFrom = path; }
  const fs::Path& RenameFrom() const { return renameFrom; }
  
  ::ftp::Control& Control() { return control; }
  ::ftp::Data& Data() { return data; }

  void SetIdleTimeout(const boost::posix_time::seconds& idleTimeout)
  { this->idleTimeout = idleTimeout; }
  const boost::posix_time::seconds& IdleTimeout() const
  { return idleTimeout; }
  
  const boost::posix_time::ptime LoggedInAt() const
  { return loggedInAt; }
  
  void SetXDupeMode(XDupeMode xdupeMode)
  { this->xdupeMode = xdupeMode; }
  ::ftp::XDupeMode XDupeMode() const { return xdupeMode; }
  
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
};

} /* ftp namespace */

#endif
