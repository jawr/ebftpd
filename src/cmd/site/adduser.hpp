#ifndef __CMD_SITE_ADDUSER_HPP
#define __CMD_SITE_ADDUSER_HPP

#include <vector>
#include <boost/optional.hpp>
#include "cmd/command.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

class ADDUSERCommand : public Command
{
  bool gadmin;
  std::string groupName;
  boost::optional<acl::User> templateUser;
  
  void Addips(const std::string& user, const std::vector<std::string>& ips);
  
public:
  ADDUSERCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args), gadmin(false) { }

  void Execute(const std::string& group, bool gadmin);
  void Execute(const acl::User& templateUser);
  void Execute();
};

// end
}
}
#endif
