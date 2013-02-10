#ifndef __CMD_SITE_DELIP_HPP
#define __CMD_SITE_DELIP_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{
class DELIPCommand : public Command
{
public:
  DELIPCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

// end
}
}
#endif

