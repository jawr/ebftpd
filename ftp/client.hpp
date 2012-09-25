#ifndef __FTP_CLIENT_HPP
#define __FTP_CLIENT_HPP

#include <string>
#include <boost/thread/mutex.hpp>
#include "acl/user.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/thread.hpp"
#include "util/error.hpp"
#include "fs/path.hpp"
#include "cmd/dirlist.hpp"

namespace ftp 
{

enum ClientState
{
  LoggedOut,
  WaitingPassword,
  LoggedIn,
  Finished,
  AnyState
};

class Client : public util::ThreadSelect
{

  mutable boost::mutex mutex;
  fs::Path workDir;
  acl::User user;
  util::net::TCPSocket control;
//  util::tcp::server dataListen;
  util::net::TCPSocket data;
  ClientState state;
  int lastCode;
  char buffer[BUFSIZ];
  std::string commandLine;
  int passwordAttemps;
  fs::Path renameFrom;
  
  static const int maxPasswordAttemps = 3;
  
  void SendReply(int code, bool part, const std::string& message);
  void DisplayWelcome();
  void NextCommand();
  void ExecuteCommand();
  void Handle();
  bool CheckState(ClientState reqdState);
  
public:
  Client() : workDir("/"), user("root", "password", "1"),
     state(LoggedOut), lastCode(0)   { }
  
  ~Client();
     
  const fs::Path& WorkDir() const { return workDir; };
  const acl::User& User() const { return user; }
  void Run();
  
  void PartReply(int code, const std::string& message);
  void PartReply(const std::string& message);
  void Reply(int code, const std::string& message);
  void Reply(const std::string& message);
  void MultiReply(int code, const std::string& messages);

  bool Accept(util::net::TCPListener& server);
  bool IsFinished() const;
  void SetFinished();
  void SetLoggedIn();
  void SetLoggedOut();
  void SetWaitingPassword();
  bool VerifyPassword(const std::string& password);
  bool PasswordAttemptsExceeded() const;
  void SetWorkDir(const std::string& workDir);
  void SetRenameFrom(const fs::Path& path) { this->renameFrom = path; }
  const fs::Path& RenameFrom() const { return renameFrom; }
  void NegotiateTLS();
  
  void DataListen();
  void DataConnect();
  
  friend cmd::DirectoryList::DirectoryList(
              ftp::Client& client, const fs::Path& path, 
              const ListOptions& options, bool dataOutput);
};

} /* ftp namespace */

#endif
