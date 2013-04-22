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

#ifndef __UTIL_PATH_HPP
#define __UTIL_PATH_HPP

#include <string>
#include <vector>

namespace util { namespace path
{

void TrimTrailingSlash(std::string& path, bool keepRootSlash = true);
std::string TrimTrailingSlashCopy(const std::string& path, bool keepRootSlash = true);
std::string Resolve(const std::string& path);
std::string Join(const std::string& path1, const std::string& path2);
std::string Append(const std::string& path1, const std::string& path2);
std::string Dirname(std::string path);
std::string Basename(std::string path);
std::string Extension(const std::string& path);
std::string NoExtension(const std::string& path);
std::string Relative(const std::string& workPath, const std::string& path);
std::vector<std::string> Glob(const std::string& path, bool tilde = true);
bool WildcardMatch(const std::string& pattern, const std::string& path, bool iCase = false);
bool Readlink(const std::string& source, std::string& dest);
bool Realpath(const std::string& source, std::string& dest);

} /* path namespace */
} /* util namespace */

#endif
