#ifndef __CMD_SITE_GRPADD_HPP
#define __CMD_SITE_GRPADD_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class GRPADDCommand : public Command
{
public:
  GRPADDCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

// end
}
}
#endif

