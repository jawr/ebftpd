#ifndef __CMD_RFC_CWD_HPP
#define __CMD_RFC_CWD_HPP

#include "cmd/command.hpp"

namespace fs
{
class VirtualPath;
class RealPath;
}

namespace cmd { namespace rfc
{

class CWDCommand : public Command
{
  void DisplayFile(const fs::RealPath& path);
  void ShowDiz(const fs::VirtualPath& path);
  void MsgPath(const fs::VirtualPath& path);
  void ShowMessage(const fs::VirtualPath& path);
  
public:
  CWDCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
