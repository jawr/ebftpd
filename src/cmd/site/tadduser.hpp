#ifndef __CMD_SITE_TADDUSER_HPP
#define __CMD_SITE_TADDUSER_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class TADDUSERCommand : public Command
{
public:
  TADDUSERCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

// end
}
}
#endif
