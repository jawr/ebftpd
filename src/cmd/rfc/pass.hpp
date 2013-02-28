#ifndef __CMD_RFC_PASS_HPP
#define __CMD_RFC_PASS_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class PASSCommand : public Command
{
public:
  PASSCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
