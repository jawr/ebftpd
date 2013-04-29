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

#ifndef __ACL_PATH_HPP
#define __ACL_PATH_HPP

#include <string>
#include "util/error.hpp"

namespace fs
{
class VirtualPath;
class Path;
}

namespace acl
{ 

class User;

namespace path
{

enum Type
{
  Upload,
  Resume,
  Overwrite,
  Makedir,
  Download,
  Rename,
  Move,
  Nuke,
  Delete,
  View,
  Hideinwho,
  Freefile,
  Nostats,
  Hideowner,
  Modify
};

template <Type type>
util::Error Allowed(const User& user, const fs::VirtualPath& path);

template <Type type>
util::Error FileAllowed(const User& user, const fs::VirtualPath& path);

template <Type type>
util::Error DirAllowed(const User& user, const fs::VirtualPath& path);

util::Error Filter(const User& user, const fs::Path& basename);

} /* path namespace */
} /* acl namespace */

#endif
