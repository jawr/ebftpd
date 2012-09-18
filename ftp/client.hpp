#ifndef __FTP_CLIENT_HPP
#define __FTP_CLIENT_HPP

#include <string>
#include "acl/user.hpp"

namespace ftp
{

class Client
{
  std::string workDir;
  acl::User user;
  
public:
  Client() : workDir("/"), user("biohazard", "somepass", "1") { }

  const std::string& WorkDir() const { return workDir; };
  const acl::User& User() const { return user; }
};

} /* ftp namespace */

#endif
