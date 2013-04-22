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

#ifndef __ACL_GROUPDATA_HPP
#define __ACL_GROUPDATA_HPP

#include <string>
#include "acl/types.hpp"

namespace acl
{

struct GroupData
{
  acl::GroupID id;
  std::string name;

  std::string description;
  std::string comment;
  
  int slots;
  int leechSlots;
  int allotmentSlots;
  long long maxAllotmentSize;
  int maxLogins;

  GroupData() :
    id(-1),
    slots(0),
    leechSlots(-2),
    allotmentSlots(-2),
    maxAllotmentSize(0),
    maxLogins(-1)
  {
  }
};

} /* acl namespace */

#endif
