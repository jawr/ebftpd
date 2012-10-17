#ifndef __CMD_SITE_USERS_HPP
#define __CMD_SITE_USERS_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class USERSCommand : public Command
{
public:
  USERSCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  cmd::Result Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif

