#ifndef __CMD_SITE_GRPCHANGE_HPP
#define __CMD_SITE_GRPCHANGE_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class GRPCHANGECommand : public Command
{
public:
  GRPCHANGECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  cmd::Result Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif

