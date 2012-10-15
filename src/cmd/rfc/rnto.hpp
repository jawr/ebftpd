#ifndef __CMD_RFC_RNTO_HPP
#define __CMD_RFC_RNTO_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class RNTOCommand : public Command
{
public:
  RNTOCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  cmd::Result Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
