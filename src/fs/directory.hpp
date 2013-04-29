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

#ifndef __FS_DIRECTORY_HPP
#define __FS_DIRECTORY_HPP

#include <string>

namespace acl
{
class User;
}

namespace util
{
class Error;
}

namespace fs
{

class VirtualPath;
class Path;
class RealPath;

util::Error CreateDirectry(const RealPath& path);
util::Error CreateDirectory(const acl::User& user, const VirtualPath& path);

util::Error RemoveDirectory(const RealPath& path);
util::Error RemoveDirectory(const acl::User& user, const VirtualPath& path);

util::Error RenameDirectory(const RealPath& oldPath, const RealPath& newPath);

util::Error ChangeAlias(const acl::User& user, const Path& path, VirtualPath& match);
util::Error ChangeMatch(const acl::User& user, const VirtualPath& path, VirtualPath& match);
util::Error ChangeCdpath(const acl::User& user, const Path& path, VirtualPath& match);
util::Error ChangeDirectory(const acl::User& user, const VirtualPath& path);

util::Error DirectorySize(const RealPath& path, int depth, long long& kBytes, 
                          bool ignoreHidden);

const VirtualPath& WorkDirectory();
void SetWorkDirectory(const VirtualPath& path);

} /* fs namespace */

#endif
