#ifndef __CMD_RFC_ALLO_HPP
#define __CMD_RFC_ALLO_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class ALLOCommand : public Command
{
public:
  ALLOCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
