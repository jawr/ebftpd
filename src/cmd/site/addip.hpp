#ifndef __CMD_SITE_ADDIP_HPP
#define __CMD_SITE_ADDIP_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class ADDIPCommand : public Command
{
public:
  ADDIPCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) {}

  void Execute();
};

// end
}
}
#endif
