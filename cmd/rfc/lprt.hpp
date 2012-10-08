#ifndef __CMD_RFC_LPRT_HPP
#define __CMD_RFC_LPRT_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class LPRTCommand : public Command
{
public:
  LPRTCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
