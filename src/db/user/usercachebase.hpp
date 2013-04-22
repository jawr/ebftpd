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

#ifndef __DB_USERCACHEBASE_HPP
#define __DB_USERCACHEBASE_HPP

#include <string>
#include "acl/types.hpp"

namespace db
{

struct UserCacheBase
{
  virtual ~UserCacheBase() { }
  virtual std::string UIDToName(acl::UserID uid) = 0;
  virtual acl::UserID NameToUID(const std::string& name) = 0;
  virtual acl::GroupID UIDToPrimaryGID(acl::UserID uid) = 0;
  virtual bool IdentIPAllowed(const std::string& identAddress) = 0;  
  virtual bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid) = 0;  
};

} /* db namespace */

#endif
