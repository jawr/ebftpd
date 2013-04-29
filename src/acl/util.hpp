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

#ifndef __ACL_UTIL_HPP
#define __ACL_UTIL_HPP

#include <string>

namespace acl
{

bool CreateDefaults();

enum class ValidationType
{
  Username,
  Groupname,
  Tagline
};

bool Validate(ValidationType type, const std::string& s);

class User;

std::string FormatRatio(int ratio);
std::string RatioString(const User& user);
std::string CreditString(const User& user);
std::string GroupString(const User& user);
std::string WeeklyAllotmentString(const User& user);

} /* acl namespace */

#endif
