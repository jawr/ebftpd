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

#ifndef __CMD_SITE_CHANGE_HPP
#define __CMD_SITE_CHANGE_HPP

#include <functional>
#include <string>
#include "cmd/command.hpp"

namespace cmd { namespace site
{

class CHANGECommand;

class CHANGECommand : public Command
{
  typedef std::function<bool(acl::User&)> SetFunction;
  typedef std::function<SetFunction(CHANGECommand*)> CheckFunction;

  struct SettingDef
  {
    std::string name;
    int maximumArgs;
    std::string aclKeyword;
    CheckFunction check;
    std::string description;
  };

  bool gadmin;
  std::string display;

  static const std::vector<SettingDef> settings;

  SetFunction CheckRatio();
  SetFunction CheckSectionRatio();
  SetFunction CheckWeeklyAllotment();
  SetFunction CheckHomeDir();
  SetFunction CheckFlags();
  SetFunction CheckIdleTime();
  SetFunction CheckExpires();
  SetFunction CheckNumLogins();
  SetFunction CheckTagline();
  SetFunction CheckComment();
  SetFunction CheckMaxUpSpeed();
  SetFunction CheckMaxDownSpeed();
  SetFunction CheckMaxSimUp();
  SetFunction CheckMaxSimDown();
  
  SetFunction Check();
  
public:
  CHANGECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args), gadmin(false) { }

  void Execute();
  
  static std::string Syntax();
};

} /* site namespace */
} /* cmd namespace */

#endif


