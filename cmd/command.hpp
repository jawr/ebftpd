#ifndef __CMD_COMMAND_HPP
#define __CMD_COMMAND_HPP

#include <vector>
#include <string>
#include <iostream>

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

class UserCommand : public Command
{
public:
  UserCommand(ftp::Client& client, const Args& args) :
    Command(client, args) { }
    
  void Execute() { std::cout << "USER command" << std::endl; }
};

class PassCommand : public Command
{
public:
  PassCommand(ftp::Client& client, const Args& args) :
    Command(client, args) { }
    
  void Execute() { std::cout << "PASS command" << std::endl; }
};

} /* cmd namespace */

#endif
