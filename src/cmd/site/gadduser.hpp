#ifndef __CMD_SITE_GADDUSER_HPP
#define __CMD_SITE_GADDUSER_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class GADDUSERCommand : public Command
{
public:
  GADDUSERCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) {}

  void Execute();
};

// end
}
}
#endif
