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

#ifndef __UTIL_PASSWD_HPP
#define __UTIL_PASSWD_HPP

#include <string>

namespace util { namespace passwd
{

extern const unsigned int defaultSaltLength;

std::string GenerateSalt(unsigned int length = defaultSaltLength);
std::string HashPassword(const std::string& password, const std::string& salt);
std::string HexEncode(const std::string& data);
std::string HexDecode(const std::string& data);

} /* passwd namespace */
} /* util namespace */

#endif