#ifndef __CMD_SITE_ADDUSER_HPP
#define __CMD_SITE_ADDUSER_HPP

#include <vector>
#include "cmd/command.hpp"

namespace cmd { namespace site
{

class ADDUSERCommand : public Command
{
  std::string group;

  void Addips(const std::string& user, const std::vector<std::string>& ips);
  
public:
  ADDUSERCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) {}

  void Execute(const std::string& group);
  void Execute();
};

// end
}
}
#endif
