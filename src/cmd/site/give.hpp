#ifndef __CMD_SITE_GIVE_HPP
#define __CMD_SITE_GIVE_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class GIVECommand : public Command
{
public:
  GIVECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  cmd::Result Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif

