#ifndef __FTP_CLIENT_HPP
#define __FTP_CLIENT_HPP

#include <string>
#include <cstdint>
#include <boost/thread/mutex.hpp>
#include "acl/user.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/net/tcplistener.hpp"
#include "util/thread.hpp"
#include "util/error.hpp"
#include "fs/path.hpp"
#include "cmd/dirlist.hpp"
#include "ftp/replycodes.hpp"
//#include "cmd/command.hpp"
#include "ftp/data.hpp"
#include "ftp/control.hpp"
/*
namespace cmd
{
class STORCommand;
class RETRCommand;
}
*/
namespace ftp 
{

enum class ClientState : uint8_t
{
  LoggedOut,
  WaitingPassword,
  LoggedIn,
  Finished,
  NotBeforeAuth,
  AnyState
};

class Client : public util::ThreadSelect
{
  mutable boost::mutex mutex;
  
  ::ftp::Control control;
  ::ftp::Data data;
  
  fs::Path workDir;
  acl::User user;
  ::ftp::ClientState state;
  int passwordAttemps;
  fs::Path renameFrom;
 
  static const int maxPasswordAttemps = 3;
  
  void DisplayBanner();
  void ExecuteCommand(const std::string& commandLine);
  void Handle();
  bool CheckState(ClientState reqdState);
  
public:
  Client() :
    data(*this), 
    workDir("/"), 
    user("root", 69, "password", "1"),
    state(ClientState::LoggedOut),
    passwordAttemps(0)
  {
  }
  
  ~Client();
     
  const fs::Path& WorkDir() const { return workDir; }
  const acl::User& User() const { return user; }
  void Run();
  
  bool Accept(util::net::TCPListener& server);
  bool IsFinished() const;
  void SetFinished();
  void SetLoggedIn();
  void SetLoggedOut();
  void SetWaitingPassword(const acl::User& user);
  bool VerifyPassword(const std::string& password);
  bool PasswordAttemptsExceeded() const;
  void SetWorkDir(const fs::Path& workDir);
  void SetRenameFrom(const fs::Path& path) { this->renameFrom = path; }
  const fs::Path& RenameFrom() const { return renameFrom; }
  
  ::ftp::Control& Control() { return control; }
  ::ftp::Data& Data() { return data; }
  
  bool IsFxp(const util::net::Endpoint& ep) const;
              
//  friend class cmd::STORCommand; // ugly
//  friend class cmd::RETRCommand; // ugly, interface needs improving so this isnt necessary
};

} /* ftp namespace */

#endif
