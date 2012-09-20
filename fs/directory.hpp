#ifndef __FS_DIRECTORY_HPP
#define __FS_DIRECTORY_HPP

#include <string>
#include "util/error.hpp"
#include "fs/path.hpp"

namespace ftp
{
class Client;
}

namespace fs
{

util::Error CreateDirectory(const ftp::Client& client, const Path& path);
util::Error RemoveDirectory(const ftp::Client& client, const Path& path);
util::Error ChangeDirectory(const ftp::Client& client, const Path& path);

} /* fs namespace */

#endif
