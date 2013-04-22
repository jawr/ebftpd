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

#ifndef __ACL_FLAGS_HPP
#define __ACL_FLAGS_HPP

#include <string>

namespace acl
{

enum class Flag : char
{
  Siteop = '1',
  Gadmin = '2',
  Template = '3',
  Exempt = '4',
  Color = '5',
  Deleted = '6',
  Useredit = '7',
  Anonymous = '8',
  Nuke = 'A',
  Unnuke = 'B',
  Undupe = 'C',
  Kick = 'D',
  Kill = 'E',
  Take = 'F',
  Give = 'G',
  Users = 'H',
  Idler = 'I',
  Custom1 = 'J',
  Custom2 = 'K',
  Custom3 = 'L',
  Custom4 = 'M',
  Custom5 = 'N'
};

bool ValidFlags(const std::string& flags);

} /* acl namespace */

#ifndef __ACL_FLAGS_CPP
extern const std::string validFlags;
#endif

#endif
