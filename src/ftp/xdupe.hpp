#ifndef __FTP_XDUPE_HPP
#define __FTP_XDUPE_HPP

#include <string>

namespace fs
{
class VirtualPath;
}

namespace ftp
{

class Client;

namespace xdupe
{

enum class Mode : unsigned
{
  Disabled = 0,
  One = 1,
  Two = 2,
  Three = 3,
  Four = 4
};

std::string Message(ftp::Client& client, const fs::VirtualPath& path);

} /* xdupe namespace */
} /* ftp namespace */

#endif
