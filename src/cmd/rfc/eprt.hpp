#ifndef __CMD_RFC_EPRT_HPP
#define __CMD_RFC_EPRT_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class EPRTCommand : public Command
{
public:
  EPRTCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
