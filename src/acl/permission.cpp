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

#include "acl/permission.hpp"
#include "acl/user.hpp"
#include "acl/acl.hpp"

namespace acl
{

Permission* new_clone(Permission const& other)
{
  return other.Clone();
}

boost::tribool FlagPermission::Evaluate(const ACLInfo& info) const
{
  if (flags.find('*') != std::string::npos) return !negate;
  for (char ch : info.flags)
  {
    if (flags.find(ch) != std::string::npos) return !negate;
  }
  return boost::indeterminate;
}

boost::tribool UserPermission::Evaluate(const ACLInfo& info) const
{
  if (info.username == username) return !negate;
  return boost::indeterminate;
}

boost::tribool GroupPermission::Evaluate(const ACLInfo& info) const
{
  if (info.groupname == groupname) return !negate;
  return boost::indeterminate;
}

} /* acl namespace */
