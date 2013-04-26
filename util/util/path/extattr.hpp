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

#ifndef __UTIL_PATH_EXTATTR_HPP
#define __UTIL_PATH_EXTATTR_HPP

#ifndef ENOATTR
# define ENOATTR ENODATA
#endif

#ifndef ENODATA
# define ENODATA ENOATTR
#endif

#if !defined(__FreeBSD__)
#include <sys/xattr.h>
#else

#include <sys/extattr.h>

inline int setxattr(const char *path, const char *name, const void *value, size_t size, int /* flags */)
{
  int ret = extattr_set_file(path, EXTATTR_NAMESPACE_USER, name, value, size);
  return ret >= 0 ? 0 : ret;
}

inline ssize_t getxattr(const char *path, const char *name, void *value, size_t size)
{
  return extattr_get_file(path, EXTATTR_NAMESPACE_USER, name, value, size);
}

inline int removexattr(const char *path, const char *name)
{
  return extattr_delete_file(path, EXTATTR_NAMESPACE_USER, name);
}

#endif
#endif
