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
  bool CheckStrings(std::string line);
  void Show(const std::string& path);
  
public:
  LOGSCommand(ftp::Client& client, const std::string& argStr, const Args& args);
  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
