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

#endif
#endif
