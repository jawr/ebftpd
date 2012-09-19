#ifndef __FTP_CLIENT_HPP
#define __FTP_CLIENT_HPP

#include <string>
#include "acl/user.hpp"
#include "util/tcpclient.hpp"

namespace ftp
{

class Client
{
  std::string workDir;
  acl::User user;
  util::tcp::client socket;
  
public:
  Client() : workDir("/"), socket(), user("biohazard", "somepass", "1") { }

  const std::string& WorkDir() const { return workDir; };
  const acl::User& User() const { return user; }

  util::tcp::client& Socket() { return socket; };
};

} /* ftp namespace */

#endif
