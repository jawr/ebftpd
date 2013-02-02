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
util::Error RenameDirectory(const acl::User& user, const VirtualPath& oldPath, 
      const VirtualPath& newPath);

util::Error ChangeAlias(const acl::User& user, const Path& path, VirtualPath& match);
util::Error ChangeMatch(const acl::User& user, const VirtualPath& path, VirtualPath& match);
util::Error ChangeCdpath(const acl::User& user, const Path& path, VirtualPath& match);
util::Error ChangeDirectory(const acl::User& user, const VirtualPath& path);

util::Error DirectorySize(const RealPath& path, int depth, long long& kBytes);

const VirtualPath& WorkDirectory();
void SetWorkDirectory(const VirtualPath& path);

} /* fs namespace */

#endif
