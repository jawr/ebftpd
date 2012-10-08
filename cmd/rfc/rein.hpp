#ifndef __CMD_RFC_REIN_HPP
#define __CMD_RFC_REIN_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class REINCommand : public Command
{
public:
  REINCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
