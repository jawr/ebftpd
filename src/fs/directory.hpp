#ifndef __FS_DIRECTORY_HPP
#define __FS_DIRECTORY_HPP

#include <string>
#include "util/error.hpp"

namespace ftp
{
class Client;
}

namespace fs
{

class VirtualPath;
class Path;
class RealPath;

util::Error CreateDirectry(const RealPath& path);
util::Error CreateDirectory(ftp::Client& client, const VirtualPath& path);

util::Error RemoveDirectory(const RealPath& path);
util::Error RemoveDirectory(ftp::Client& client, const VirtualPath& path);

util::Error RenameDirectory(const RealPath& oldPath, const RealPath& newPath);
util::Error RenameDirectory(ftp::Client& client, const VirtualPath& oldPath, 
      const VirtualPath& newPath);

util::Error ChangeAlias(ftp::Client& client, const Path& path, VirtualPath& match);
util::Error ChangeMatch(ftp::Client& client, const VirtualPath& path, VirtualPath& match);
util::Error ChangeCdpath(ftp::Client& client, const Path& path, VirtualPath& match);
util::Error ChangeDirectory(ftp::Client& client, const VirtualPath& path);

util::Error DirectorySize(const RealPath& path, int depth, long long& kBytes);

const VirtualPath& WorkDirectory();
void SetWorkDirectory(const VirtualPath& path);

} /* fs namespace */

#endif
