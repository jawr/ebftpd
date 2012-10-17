#ifndef __CMD_SITE_LOGS_HPP
#define __CMD_SITE_LOGS_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class LOGSCommand : public Command
{
  std::string log;
  int number;
  std::vector<std::string> strings;
  
  static const int defaultNumberLines = 100;
  
  bool ParseArgs();
  bool CheckStrings(const std::string& line);
  void Show(const std::string& path);
  
public:
  LOGSCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args),
    number(defaultNumberLines) { }

  cmd::Result Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
