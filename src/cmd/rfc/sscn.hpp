#ifndef __CMD_RFC_SSCN_HPP
#define __CMD_RFC_SSCN_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class SSCNCommand : public Command
{
public:
  SSCNCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  cmd::Result Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
