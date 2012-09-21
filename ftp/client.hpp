#ifndef __FTP_CLIENT_HPP
#define __FTP_CLIENT_HPP

#include <string>
#include <boost/thread/mutex.hpp>
#include "acl/user.hpp"
#include "util/tcpclient.hpp"
#include "util/thread.hpp"
#include "util/error.hpp"

namespace ftp 
{

class Client : public util::ThreadSelect
{
  mutable boost::mutex mutex;
  std::string workDir;
  acl::User user;
  util::tcp::client socket;
  bool finished;
  int lastCode;
  char buffer[BUFSIZ];
  std::string command;

  void SendReply(int code, bool part, const std::string& message);
  void DisplayWelcome();
  void NextCommand();
  void Handle();
  
public:
  Client() : workDir("/"), user("biohazard", "somepass", "1"), lastCode(0)   { }

  const std::string& WorkDir() const { return workDir; };
  const acl::User& User() const { return user; }
  void Run();
  
  void PartReply(int code, const std::string& message);
  void PartReply(const std::string& message);
  void Reply(int code, const std::string& message);
  void Reply(const std::string& message);

  bool Accept(util::tcp::server& server);
  bool Finished() const;
};

} /* ftp namespace */

#endif
