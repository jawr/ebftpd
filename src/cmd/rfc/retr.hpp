#ifndef __CMD_RFC_RETR_HPP
#define __CMD_RFC_RETR_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class RETRCommand : public Command
{
public:
  RETRCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  cmd::Result Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
