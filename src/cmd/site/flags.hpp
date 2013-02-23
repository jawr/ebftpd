#ifndef __CMD_SITE_FLAGS_HPP
#define __CMD_SITE_FLAGS_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class FLAGSCommand : public Command
{ 
public:
  FLAGSCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

// end
}
}
#endif

