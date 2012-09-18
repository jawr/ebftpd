#ifndef __FS_DIRECTORY_HPP
#define __FS_DIRECTORY_HPP

#include <string>
#include "error.hpp"

namespace ftp
{
class Client;
}

namespace fs
{

Error CreateDirectory(const ftp::Client& client, const std::string& path);
Error RemoveDirectory(const ftp::Client& client, const std::string& path);
Error ChangeDirectory(const ftp::Client& client, const std::string& path);

} /* fs namespace */

#endif
