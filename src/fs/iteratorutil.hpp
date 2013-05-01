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

#ifndef __FS_ITERATOR_UTIL_HPP
#define __FS_ITERATOR_UTIL_HPP

#include <iostream>

namespace fs
{

inline bool Filter(const acl::User& user, const std::string& path)
{
  return !acl::path::Allowed<acl::path::View>(user, MakeVirtual(fs::RealPath(path)));
}

inline std::string PreFilter(const acl::User& user, const std::string& path)
{
  RealPath real = MakeReal(fs::Path(path));
  if (Filter(user, real.ToString())) throw util::SystemError(ENOENT);
  return real.ToString();
}

} /* fs namespace */

#endif
