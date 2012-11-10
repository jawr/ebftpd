#ifndef __CMD_SITE_MSG_HPP
#define __CMD_SITE_MSG_HPP

#include <vector>
#include "cmd/command.hpp"

namespace db { namespace mail
{
class Message;
} 
}

namespace cmd { namespace site
{

class MSGCommand : public Command
{
  void Read(const std::vector<db::mail::Message>& mail);
  cmd::Result Read();
  cmd::Result Send();
  cmd::Result SaveTrash();
  cmd::Result Save();
  cmd::Result PurgeTrash();
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
