#ifndef __CMD_SITE_CHANGE_HPP
#define __CMD_SITE_CHANGE_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class CHANGECommand : public Command
{
public:
  CHANGECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif


