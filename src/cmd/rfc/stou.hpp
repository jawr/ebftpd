#ifndef __CMD_RFC_STOU_HPP
#define __CMD_RFC_STOU_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class STOUCommand : public Command
{
public:
  STOUCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
