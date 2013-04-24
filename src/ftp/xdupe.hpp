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

#ifndef __FTP_XDUPE_HPP
#define __FTP_XDUPE_HPP

#include <string>

namespace fs
{
class VirtualPath;
}

namespace ftp
{

class Client;

namespace xdupe
{

enum class Mode : unsigned
{
  Disabled = 0,
  One = 1,
  Two = 2,
  Three = 3,
  Four = 4
};

std::string Message(ftp::Client& client, const fs::VirtualPath& path);

} /* xdupe namespace */
} /* ftp namespace */

#endif
