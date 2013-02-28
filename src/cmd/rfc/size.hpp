#ifndef __CMD_RFC_SIZE_HPP
#define __CMD_RFC_SIZE_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class SIZECommand : public Command
{
public:
  SIZECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
