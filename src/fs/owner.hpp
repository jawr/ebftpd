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

#ifndef __FS_OWNER_HPP
#define __FS_OWNER_HPP

#include <ostream>
#include "acl/types.hpp"
#include "fs/path.hpp"

namespace util
{
class Error;
}

namespace fs
{

class Owner
{
  acl::UserID uid;
  acl::GroupID gid;
  
public:
  Owner(acl::UserID uid, acl::GroupID gid) : uid(uid), gid(gid) { }
  
  acl::UserID UID() const { return uid; }
  acl::GroupID GID() const { return gid; }
};

Owner GetOwner(const std::string& path);
util::Error SetOwner(const std::string& path, const Owner& owner);

Owner GetOwner(const RealPath& path);
util::Error SetOwner(const RealPath& path, const Owner& owner);

inline std::ostream& operator<<(std::ostream& os, const Owner& owner)
{
  os << owner.UID() << "," << owner.GID();
  return os;
}

} /* fs namespace */



#endif
