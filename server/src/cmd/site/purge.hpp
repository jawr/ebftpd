#ifndef __CMD_SITE_PURGE_HPP
#define __CMD_SITE_PURGE_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class PURGECommand : public Command
{
public:
  PURGECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
