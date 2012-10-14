#ifndef __CMD_SITE_RENUSER_HPP
#define __CMD_SITE_RENUSER_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class RENUSERCommand : public Command
{
public:
  RENUSERCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) {}

  void Execute();
};

// end
}
}
#endif
