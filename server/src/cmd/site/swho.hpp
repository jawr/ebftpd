#ifndef __CMD_SITE_SWHO_HPP
#define __CMD_SITE_SWHO_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class SWHOCommand : public Command
{
public:
  SWHOCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif

