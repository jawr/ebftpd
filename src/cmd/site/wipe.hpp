#ifndef __CMD_SITE_WIPE_HPP
#define __CMD_SITE_WIPE_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class WIPECommand : public Command
{
  std::string pathmask;
  bool recursive;
  int dirs;
  int files;
  int failed;
  
  void Process(const fs::Path& absmask, int dept = 1);
  bool ParseArgs();
  
public:
  WIPECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args),
    recursive(false), dirs(0), files(0), failed(0) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
