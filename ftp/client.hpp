#ifndef __FTP_CLIENT_HPP
#define __FTP_CLIENT_HPP

#include <string>
#include <boost/thread/mutex.hpp>
#include "acl/user.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/net/tcplistener.hpp"
#include "util/thread.hpp"
#include "util/error.hpp"
#include "fs/path.hpp"
#include "cmd/dirlist.hpp"
#include "ftp/replycodes.hpp"

namespace cmd
{
class STORCommand;
class RETRCommand;
}

namespace ftp 
{

namespace ClientState
{
  enum Enum
  {
    LoggedOut,
    WaitingPassword,
    LoggedIn,
    Finished,
    NotBeforeAuth,
    AnyState
  };
};

namespace EPSVMode
{
  enum Enum
  {
    Normal,
    Full
  };
};

namespace DataType
{
  enum Enum
  {
    ASCII,
    Binary
  };
};

class Client : public util::ThreadSelect
{

  mutable boost::mutex mutex;
  fs::Path workDir;
  acl::User user;
  util::net::TCPSocket control;
  ::ftp::ClientState::Enum state;
  ReplyCode lastCode;
  std::string commandLine;
  int passwordAttemps;
  fs::Path renameFrom;

  // data connections
  util::net::TCPListener dataListen;
  util::net::TCPSocket data;
  bool dataProtected;
  bool passiveMode;
  util::net::Endpoint portEndpoint;
  ::ftp::EPSVMode::Enum epsvMode;
  ::ftp::DataType::Enum dataType;
  
  static const int maxPasswordAttemps = 3;
  
  void SendReply(ReplyCode code, bool part, const std::string& message);
  void DisplayWelcome();
  void NextCommand();
  void ExecuteCommand();
  void Handle();
  bool CheckState(ClientState::Enum reqdState);
  
public:
  Client() : workDir("/"), user("root", 69, "password", "1"),
     control(15), state(ClientState::LoggedOut), lastCode(CodeNotSet),
     passwordAttemps(0), data(15), dataProtected(false),
     passiveMode(false), epsvMode(::ftp::EPSVMode::Normal) { }
  
  ~Client();
     
  const fs::Path& WorkDir() const { return workDir; };
  const acl::User& User() const { return user; }
  void Run();
  
  void PartReply(ReplyCode code, const std::string& message);
  void PartReply(const std::string& message);
  void Reply(ReplyCode code, const std::string& message);
  void Reply(const std::string& message);
  void MultiReply(ReplyCode code, const std::string& messages);

  bool Accept(util::net::TCPListener& server);
  bool IsFinished() const;
  void SetFinished();
  void SetLoggedIn();
  void SetLoggedOut();
  void SetWaitingPassword();
  bool VerifyPassword(const std::string& password);
  bool PasswordAttemptsExceeded() const;
  void SetWorkDir(const fs::Path& workDir);
  void SetRenameFrom(const fs::Path& path) { this->renameFrom = path; }
  const fs::Path& RenameFrom() const { return renameFrom; }
  void NegotiateTLS();
  
  bool DataProtected() const { return dataProtected; }
  void SetDataProtected(bool dataProtected) { this->dataProtected = dataProtected; }

  ::ftp::EPSVMode::Enum EPSVMode() const { return epsvMode; }
  void SetEPSVMode(::ftp::EPSVMode::Enum epsvMode) { this->epsvMode = epsvMode; }

  ::ftp::DataType::Enum DataType() const { return dataType; }
  void SetDataType(::ftp::DataType::Enum dataType) { this->dataType = dataType; }
  
  void DataInitPassive(util::net::Endpoint& ep, bool ipv4Only);
  void DataInitActive(const util::net::Endpoint& ep);
  void DataOpen();
  void DataClose() { data.Close(); }
  
  friend cmd::DirectoryList::DirectoryList(
              ftp::Client& client, const fs::Path& path, 
              const ListOptions& options, bool dataOutput, int maxRecursion);
              
  friend class cmd::STORCommand; // ugly
  friend class cmd::RETRCommand; // ugly, interface needs improving so this isnt necessary
};

} /* ftp namespace */

#endif
