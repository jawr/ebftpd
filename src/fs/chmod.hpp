#ifndef __FS_CHMOD_HPP
#define __FS_CHMOD_HPP

#include <string>
#include <sys/types.h>
#include "util/error.hpp"
#include "fs/path.hpp"

namespace ftp
{
class Client;
}

namespace fs
{

util::Error ModeFromString(const std::string& str, mode_t& mode);
util::Error Chmod(ftp::Client& client, const fs::Path& path, mode_t mode);

} /* fs namespace */

#endif
