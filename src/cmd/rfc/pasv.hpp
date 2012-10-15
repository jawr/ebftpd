#ifndef __CMD_RFC_PASV_HPP
#define __CMD_RFC_PASV_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class PASVCommand : public Command
{
public:
  PASVCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  cmd::Result Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
