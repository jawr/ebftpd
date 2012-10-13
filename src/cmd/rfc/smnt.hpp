#ifndef __CMD_RFC_SMNT_HPP
#define __CMD_RFC_SMNT_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class SMNTCommand : public Command
{
public:
  SMNTCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
