#ifndef __CMD_RFC_STRU_HPP
#define __CMD_RFC_STRU_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class STRUCommand : public Command
{
public:
  STRUCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
