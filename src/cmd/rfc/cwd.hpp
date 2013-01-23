#ifndef __CMD_RFC_CWD_HPP
#define __CMD_RFC_CWD_HPP

#include "cmd/command.hpp"

namespace fs
{
class VirtualPath;
}

namespace cmd { namespace rfc
{

class CWDCommand : public Command
{
  void ShowDiz(const fs::VirtualPath& path);
  
public:
  CWDCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
