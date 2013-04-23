#ifndef __CMD_SITE_NUKING_HPP
#define __CMD_SITE_NUKING_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class NUKECommand : public Command
{
public:
  NUKECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

class UNNUKECommand : public Command
{
public:
  UNNUKECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

class NUKESCommand : public Command
{
public:
  NUKESCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
