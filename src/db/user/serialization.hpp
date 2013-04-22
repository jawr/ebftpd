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

#ifndef __DB_USER_SERIALIZATION_HPP
#define __DB_USER_SERIALIZATION_HPP

#include <mongo/client/dbclient.h>
#include <string>
#include "acl/types.hpp"

namespace db
{

struct UserTriple
{
  std::string name;
  acl::UserID uid;
  acl::GroupID primaryGid;
};

template <typename T> T Unserialize(const mongo::BSONObj& obj);

template <> inline UserTriple Unserialize<UserTriple>(const mongo::BSONObj& obj)
{
  UserTriple data;
  data.name = obj["name"].String();
  data.uid = obj["uid"].Int();
  data.primaryGid = obj["primary gid"].Int();
  return data;
}

} /* db namespace */

#endif
