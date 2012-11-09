#ifndef __CMD_SITE_SREPLY_HPP
#define __CMD_SITE_SREPLY_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class SREPLYCommand : public Command
{
public:
  SREPLYCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  cmd::Result Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
