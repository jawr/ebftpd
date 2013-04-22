//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
