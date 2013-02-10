#ifndef __FS_CHMOD_HPP
#define __FS_CHMOD_HPP

#include <string>
#include <sys/types.h>

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

class Mode;
class VirtualPath;
class RealPath;

util::Error Chmod(const RealPath& path, const Mode& mode);
util::Error Chmod(const acl::User& user, const VirtualPath& path, const Mode& mode);

} /* fs namespace */

#endif
