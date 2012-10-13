#ifndef __CMD_RFC_APPE_HPP
#define __CMD_RFC_APPE_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class APPECommand : public Command
{
public:
  APPECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
