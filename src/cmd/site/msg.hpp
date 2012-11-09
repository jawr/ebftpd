#ifndef __CMD_SITE_MSG_HPP
#define __CMD_SITE_MSG_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class MSGCommand : public Command
{
  cmd::Result Read(bool trash = false);
  cmd::Result Send();
  cmd::Result Save();
  cmd::Result Purge();
  cmd::Result List();
  
public:
  MSGCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  cmd::Result Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
