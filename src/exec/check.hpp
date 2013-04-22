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

#ifndef __EXEC_CHECK_HPP
#define __EXEC_CHECK_HPP

#include <string>

namespace ftp
{
class Client;
}

namespace fs
{
class VirtualPath;
}

namespace exec
{

bool PreCheck(ftp::Client& client, const fs::VirtualPath& path);
bool PreDirCheck(ftp::Client& client, const fs::VirtualPath& path);
bool PostCheck(ftp::Client& client, const fs::VirtualPath& path, const std::string& crc,
      double speed, const std::string& section);

} /* exec namespace */

#endif
