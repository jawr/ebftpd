#ifndef __CMD_SITE_CHPASS_HPP
#define __CMD_SITE_CHPASS_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class CHPASSCommand : public Command
{
public:
  CHPASSCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  cmd::Result Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
