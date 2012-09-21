#ifndef __CMD_COMMAND_HPP
#define __CMD_COMMAND_HPP

#include <vector>
#include <string>

namespace ftp
{
class Client;
}

namespace cmd
{
typedef std::vector<std::string> Args;

class Command
{
  ftp::Client& client;
  Args args;
  
public:
  Command(ftp::Client& client, const Args& args) :
    client(client), args(args) { }
    
  virtual void Execute() = 0;
};

} /* cmd namespace */

#endif
