#ifndef __CMD_RFC_RMD_HPP
#define __CMD_RFC_RMD_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class RMDCommand : public Command
{
public:
  RMDCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
