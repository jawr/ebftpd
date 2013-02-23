#ifndef __CMD_RFC_COMMAND_HPP
#define __CMD_RFC_COMMAND_HPP

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include "ftp/client.hpp"
#include "ftp/control.hpp"

namespace ftp
{
class Control;
class Data;
}

namespace cmd
{

typedef std::vector<std::string> Args;

class Command
{
protected:
  ftp::Client& client;
  ftp::Control& control;
  ftp::Data& data;
  std::string argStr;
  Args args;
  
public:
  Command(ftp::Client& client, ftp::Control& control, ftp::Data& data, 
          const std::string argStr, const Args& args) :
    client(client), control(control), data(data), argStr(argStr), args(args) { }

  virtual ~Command() { }
    
  virtual void Execute() = 0;
};

typedef std::shared_ptr<Command> CommandPtr;

} /* cmd namespace */

#endif
