#ifndef __FS_CHMOD_HPP
#define __FS_CHMOD_HPP

#include <string>
#include <sys/types.h>
#include "util/error.hpp"

namespace ftp
{
class Client;
}

namespace fs
{

class Mode;
class VirtualPath;
class RealPath;

util::Error Chmod(const RealPath& path, const Mode& mode);
util::Error Chmod(ftp::Client& client, const VirtualPath& path, const Mode& mode);

} /* fs namespace */

#endif
