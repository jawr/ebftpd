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

#ifndef __ACL_ACL_HPP
#define __ACL_ACL_HPP

#include <string>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/optional.hpp>
#include "acl/permission.hpp"

namespace acl
{

class User;

struct ACLInfo
{
  std::string username;
  std::string groupname;
  std::string flags;
  
  ACLInfo(const std::string& username, 
          const std::string& groupname, 
          const std::string& flags) :
    username(username),
    groupname(groupname),
    flags(flags)
  {
  }
};

class ACL
{
  boost::ptr_vector<Permission> perms;
  mutable boost::optional<bool> finalResult;

  void FromStringArg(const std::string& arg);
  void FromString(const std::string& str);
  
public:
  ACL() = default;
  ACL(const std::string& s);
 
  bool Evaluate(const ACLInfo& info) const;

  bool Evaluate(const std::string& username, 
                const std::string& groupname, 
                const std::string& flags) const
  {
    return Evaluate(ACLInfo(username, groupname, flags));
  }
};

} /* acl namespace */

#endif
