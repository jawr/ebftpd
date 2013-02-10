#ifndef __CMD_SITE_RANKS_HPP
#define __CMD_SITE_RANKS_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class RANKSCommand : public Command
{
public:
  RANKSCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
