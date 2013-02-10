#ifndef __CMD_RFC_CPSV_HPP
#define __CMD_RFC_CPSV_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class CPSVCommand : public Command
{
public:
  CPSVCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
