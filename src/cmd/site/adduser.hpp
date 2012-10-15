#ifndef __CMD_SITE_ADDUSER_HPP
#define __CMD_SITE_ADDUSER_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class ADDUSERCommand : public Command
{
public:
  ADDUSERCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) {}

  cmd::Result Execute();
};

// end
}
}
#endif
