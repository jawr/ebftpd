#ifndef __CMD_SITE_READD_HPP
#define __CMD_SITE_READD_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class READDCommand : public Command
{
public:
  READDCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) {}

  void Execute();
};

// end
}
}
#endif
