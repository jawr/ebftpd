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

#ifndef __ACL_TYPES_HPP
#define __ACL_TYPES_HPP

#include <cstdint>

namespace acl
{

/*class GroupID;

class UserID
{
  int32_t id;

public:
  UserID() : id(-1) { }
  UserID(int32_t id) : id(id) { }
  operator int32_t() const { return id; }
};

class GroupID
{
  int32_t id;

public:
  GroupID() : id(-1) { }
  GroupID(int32_t id) : id(id) { }
  operator int32_t() const { return id; }
};
*/
typedef int32_t UserID;
typedef int32_t GroupID;

} /* acl namespace */

#endif
