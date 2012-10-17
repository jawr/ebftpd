#ifndef __CMD_SITE_WIPE_HPP
#define __CMD_SITE_WIPE_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class WIPECommand : public Command
{
  std::string pathmaskStr;
  bool recursive;
  int dirs;
  int files;
  int failed;
  
  void Process(const fs::Path& pathmask, int dept = 1);
  bool ParseArgs();
  
public:
  WIPECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args),
    recursive(false), dirs(0), files(0), failed(0) { }

  cmd::Result Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
