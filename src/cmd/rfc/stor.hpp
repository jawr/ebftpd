#ifndef __CMD_RFC_STOR_HPP
#define __CMD_RFC_STOR_HPP

#include "cmd/command.hpp"

namespace cmd { namespace rfc
{

class STORCommand : public Command
{
  bool CalcCRC(const fs::Path& path);

  
public:
  STORCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
