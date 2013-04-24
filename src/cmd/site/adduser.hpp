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

#ifndef __CMD_SITE_ADDUSER_HPP
#define __CMD_SITE_ADDUSER_HPP

#include <vector>
#include <boost/optional.hpp>
#include "cmd/command.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

class ADDUSERCommand : public Command
{
  bool gadmin;
  std::string groupName;
  boost::optional<acl::User> templateUser;
  
  void Addips(const std::string& user, const std::vector<std::string>& ips);
  
public:
  ADDUSERCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args), gadmin(false) { }

  void Execute(const std::string& group, bool gadmin);
  void Execute(const acl::User& templateUser);
  void Execute();
};

// end
}
}
#endif
