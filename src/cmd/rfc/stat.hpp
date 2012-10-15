#ifndef __CMD_RFC_STAT_HPP
#define __CMD_RFC_STAT_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class STATCommand : public Command
{
public:
  STATCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  cmd::Result Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
