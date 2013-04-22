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

#ifndef __CMD_RFC_CWD_HPP
#define __CMD_RFC_CWD_HPP

#include "cmd/command.hpp"

namespace fs
{
class VirtualPath;
class RealPath;
}

namespace cmd { namespace rfc
{

class CWDCommand : public Command
{
  void DisplayFile(const fs::RealPath& path);
  void ShowDiz(const fs::VirtualPath& path);
  void MsgPath(const fs::VirtualPath& path);
  void ShowMessage(const fs::VirtualPath& path);
  
public:
  CWDCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
