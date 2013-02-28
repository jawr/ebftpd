#ifndef __CMD_RFC_LIST_HPP
#define __CMD_RFC_LIST_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class LISTCommand : public Command
{
  bool nlst;

public:
  LISTCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args), nlst(false) { }

  void ExecuteNLST();
  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
