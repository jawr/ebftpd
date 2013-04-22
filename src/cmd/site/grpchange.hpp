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

#ifndef __CMD_SITE_GRPCHANGE_HPP
#define __CMD_SITE_GRPCHANGE_HPP

#include <functional>
#include <string>
#include "cmd/command.hpp"

namespace acl
{
class Group;
}

namespace cmd { namespace site
{

class GRPCHANGECommand;

class GRPCHANGECommand : public Command
{
  typedef std::function<void(acl::Group&)> SetFunction;
  typedef std::function<SetFunction(GRPCHANGECommand*)> CheckFunction;

  struct SettingDef
  {
    std::string name;
    int maximumArgs;
    std::string aclKeyword;
    CheckFunction check;
    std::string description;
  };
  
  std::string display;

  static const std::vector<SettingDef> settings;
  
  SetFunction CheckSlots();
  SetFunction CheckLeechSlots();
  SetFunction CheckAllotSlots();
  SetFunction CheckMaxAllotSize();
  SetFunction CheckMaxLogins();
  SetFunction CheckDescription();
  SetFunction CheckComment();
  
  SetFunction Check();

public:
  GRPCHANGECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
  
  static std::string Syntax();
};

} /* site namespace */
} /* cmd namespace */

#endif

