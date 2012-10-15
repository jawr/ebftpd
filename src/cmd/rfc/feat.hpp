#ifndef __CMD_RFC_FEAT_HPP
#define __CMD_RFC_FEAT_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class FEATCommand : public Command
{
public:
  FEATCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  cmd::Result Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
