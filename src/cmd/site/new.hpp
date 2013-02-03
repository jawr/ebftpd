#ifndef __CMD_SITE_NEW_HPP
#define __CMD_SITE_NEW_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class NEWCommand : public Command
{
public:
  NEWCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif