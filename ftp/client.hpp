#ifndef __FTP_CLIENT_HPP
#define __FTP_CLIENT_HPP

#include <string>
#include <cstdint>
#include <memory>
#include <boost/thread/mutex.hpp>
#include "acl/user.hpp"
#include "util/net/tcpsocket.hpp"
#include "util/net/tcplistener.hpp"
#include "util/thread.hpp"
#include "util/error.hpp"
#include "fs/path.hpp"
#include "cmd/dirlist.hpp"
#include "ftp/replycodes.hpp"
#include "cmd/command.hpp"

namespace cmd
{
class STORCommand;
class RETRCommand;
}

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

enum class EPSVMode : uint8_t
{
  Normal,
  Full
};

enum class DataType : uint8_t
{
  ASCII,
  Binary
};

enum class TransferType : uint8_t
{
  Upload,
  Download,
  List
};

enum class PassiveType : uint8_t
{
  PASV,
  EPSV,
  LPSV
};

class Client : public util::ThreadSelect
{

  mutable boost::mutex mutex;
  fs::Path workDir;
  std::shared_ptr<acl::User> user;
  util::net::TCPSocket control;
  ::ftp::ClientState state;
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
  ::ftp::EPSVMode epsvMode;
  ::ftp::DataType dataType;
  
  static const int maxPasswordAttemps = 3;
  
  void SendReply(ReplyCode code, bool part, const std::string& message);
  void DisplayWelcome();
  void NextCommand();
  void ExecuteCommand();
  void Handle();
  bool CheckState(ClientState reqdState);
  
public:
  Client() : workDir("/"), user(new acl::User()),
     control(15), state(ClientState::LoggedOut), lastCode(CodeNotSet),
     passwordAttemps(0), data(15), dataProtected(false),
     passiveMode(false), epsvMode(::ftp::EPSVMode::Normal) { }
  
  ~Client();
     
<<<<<<< HEAD
  const fs::Path& WorkDir() const { return workDir; }
  const acl::User& User() const { return user; }
=======
  const fs::Path& WorkDir() const { return workDir; };
  const acl::User& User() const { return *user; }
>>>>>>> Tied main.cpp into the db as well as Client (had to modify UserCache to be able to return a non const pointer, also changed Client to have a shared_ptr - worried that even if the client goes out of scope it will delete the pointer as the ref count isn't counted int he container in the cache, might have to store it as a raw pointer instead)
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

  ::ftp::EPSVMode EPSVMode() const { return epsvMode; }
  void SetEPSVMode(::ftp::EPSVMode epsvMode) { this->epsvMode = epsvMode; }

  ::ftp::DataType DataType() const { return dataType; }
  void SetDataType(::ftp::DataType dataType) { this->dataType = dataType; }
  
  void DataInitPassive(util::net::Endpoint& ep, PassiveType pasvType);
  void DataInitActive(const util::net::Endpoint& ep);
  void DataOpen(TransferType transferType);
  void DataClose() { data.Close(); }
  
  friend cmd::DirectoryList::DirectoryList(
              ftp::Client& client, const fs::Path& path, 
              const ListOptions& options, bool dataOutput, int maxRecursion);
              
  bool IsFxp(const util::net::Endpoint& ep) const;
 
  friend class cmd::USERCommand; // ugly...             
  friend class cmd::STORCommand; // ugly
  friend class cmd::RETRCommand; // ugly, interface needs improving so this isnt necessary
};

} /* ftp namespace */

#endif
