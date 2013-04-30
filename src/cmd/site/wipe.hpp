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

#ifndef __CMD_SITE_WIPE_HPP
#define __CMD_SITE_WIPE_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class WIPECommand : public Command
{
  std::string patharg;
  bool recursive;
  int dirs;
  int files;
  int failed;
  
  void Process(const fs::VirtualPath& pathmask);
  void ParseArgs();
  
public:
  WIPECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args),
    recursive(false), dirs(0), files(0), failed(0) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
