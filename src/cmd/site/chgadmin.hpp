#ifndef __CMD_SITE_CHGADMIN_HPP
#define __CMD_SITE_CHGADMIN_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class CHGADMINCommand : public Command
{
public:
  CHGADMINCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
