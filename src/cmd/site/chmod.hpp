#ifndef __CMD_SITE_CHMOD_HPP
#define __CMD_SITE_CHMOD_HPP

#include <boost/optional.hpp>
#include "cmd/command.hpp"
#include "fs/mode.hpp"

namespace cmd { namespace site
{

class CHMODCommand : public Command
{
  boost::optional<fs::Mode> mode;
  std::string patharg;
  std::string modeStr;
  bool recursive;
  int dirs;
  int files;
  int failed;
  
  void Process(fs::VirtualPath pathmask);
  void ParseArgs();
  
public:
  CHMODCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args),
    recursive(false), dirs(0), files(0), failed(0) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
