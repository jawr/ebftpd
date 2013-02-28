#ifndef __CMD_SITE_GOODBYE_HPP
#define __CMD_SITE_GOODBYE_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class GOODBYECommand : public Command
{
public:
  GOODBYECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
