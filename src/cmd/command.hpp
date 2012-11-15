#ifndef __CMD_RFC_COMMAND_HPP
#define __CMD_RFC_COMMAND_HPP

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include "ftp/client.hpp"

namespace ftp
{
class Control;
class Data;
}

namespace cmd
{

enum class Result : int
{
  SyntaxError,
  Finished,
  Permission,
  Okay
};

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
    
  virtual Result Execute() = 0;
};

typedef std::shared_ptr<Command> CommandPtr;

} /* cmd namespace */

#endif
