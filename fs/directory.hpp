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

util::Error CreateDirectory(ftp::Client& client, const Path& path);
util::Error RemoveDirectory(ftp::Client& client, const Path& path);
util::Error ChangeDirectory(ftp::Client& client, Path& path);

} /* fs namespace */

#endif
