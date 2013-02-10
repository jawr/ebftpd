#ifndef __EXEC_UTIL_HPP
#define __EXEC_UTIL_HPP

#include <string>
#include <vector>

namespace ftp
{
class Client;
}

namespace exec
{

std::vector<std::string> BuildEnv(ftp::Client& client);

} /* exec namespace */

#endif
