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

#ifndef __CMD_SITE_MSG_HPP
#define __CMD_SITE_MSG_HPP

#include <vector>
#include "cmd/command.hpp"

namespace db { namespace mail
{
class Message;
} 
}

namespace cmd { namespace site
{

class MSGCommand : public Command
{
  void Read(const std::vector<db::mail::Message>& mail);
  void Read();
  void Send();
  void SaveTrash();
  void Save();
  void PurgeTrash();
  void Purge();
  void List();
  
public:
  MSGCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
