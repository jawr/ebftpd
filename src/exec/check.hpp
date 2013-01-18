#ifndef __EXEC_CHECK_HPP
#define __EXEC_CHECK_HPP

#include <string>

namespace ftp
{
class Client;
}

namespace fs
{
class VirtualPath;
}

namespace exec
{

bool PreCheck(ftp::Client& client, const fs::VirtualPath& path);
bool PreDirCheck(ftp::Client& client, const fs::VirtualPath& path);
bool PostCheck(ftp::Client& client, const fs::VirtualPath& path, const std::string& crc,
      double speed, const std::string& section);

} /* exec namespace */

#endif
