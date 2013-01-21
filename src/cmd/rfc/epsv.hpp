#ifndef __CMD_RFC_EPSV_HPP
#define __CMD_RFC_EPSV_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class EPSVCommand : public Command
{
  void ChangeMode();
  
public:
  EPSVCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
