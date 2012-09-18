#ifndef __FS_DIRECTORY_HPP
#define __FS_DIRECTORY_HPP

#include <string>
#include "util/error.hpp"
#include "path.hpp"

namespace ftp
{
class Client;
}

namespace fs
{

Error CreateDirectory(const ftp::Client& client, const Path& path);
Error RemoveDirectory(const ftp::Client& client, const Path& path);
Error ChangeDirectory(const ftp::Client& client, const Path& path);

} /* fs namespace */

#endif
