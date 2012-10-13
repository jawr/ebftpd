#ifndef __CMD_RFC_OPTS_HPP
#define __CMD_RFC_OPTS_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class OPTSCommand : public Command
{
public:
  OPTSCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
