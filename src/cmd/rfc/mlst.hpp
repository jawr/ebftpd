#ifndef __CMD_RFC_MLST_HPP
#define __CMD_RFC_MLST_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class MLSTCommand : public Command
{
public:
  MLSTCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
