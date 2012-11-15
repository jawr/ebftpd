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
  void Read();
  void Send();
  void SaveTrash();
  void Save();
  void PurgeTrash();
  void Purge();
  void List();
  
public:
  MSGCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
