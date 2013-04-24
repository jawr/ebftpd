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

#ifndef __DB_GROUP_SERIALIZATION_HPP
#define __DB_GROUP_SERIALIZATION_HPP

#include <mongo/client/dbclient.h>
#include <string>
#include "acl/types.hpp"

namespace db
{

struct GroupPair
{
  std::string name;
  acl::GroupID gid;
};

template <typename T> T Unserialize(const mongo::BSONObj& obj);
template <> inline GroupPair Unserialize<GroupPair>(const mongo::BSONObj& obj)
{
  GroupPair data;
  data.name = obj["name"].String();
  data.gid = obj["gid"].Int();
  return data;
}

} /* db namespace */

#endif
