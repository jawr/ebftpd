#ifndef __FTP_CLIENT_HPP
#define __FTP_CLIENT_HPP

#include <string>
#include "acl/user.hpp"
#include "util/tcpclient.hpp"
#include "util/thread.hpp"

namespace ftp 
{

class Client : public util::Thread
{
  std::string workDir;
  acl::User user;
  util::tcp::client socket;
  
public:
  Client() : Thread(), workDir("/"), user("biohazard", "somepass", "1") { }

  const std::string& WorkDir() const { return workDir; };
  const acl::User& User() const { return user; }

  util::tcp::client& Socket() { return socket; };
  void Run() { };
};

} /* ftp namespace */

#endif
