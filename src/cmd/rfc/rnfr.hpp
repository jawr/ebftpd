#ifndef __CMD_RFC_RNFR_HPP
#define __CMD_RFC_RNFR_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class RNFRCommand : public Command
{
public:
  RNFRCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
