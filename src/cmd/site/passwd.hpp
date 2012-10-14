#ifndef __CMD_PASSWD_ABOR_HPP
#define __CMD_PASSWD_ABOR_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class PASSWDCommand : public Command
{
public:
  PASSWDCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
