#ifndef __CMD_UTIL_HPP
#define __CMD_UTIL_HPP

#include <string>
#include <vector>

namespace cmd
{

bool ParseCredits(std::string s, long long& credits);
bool SplitArgs(const std::string& command, std::vector<std::string>& args);

} /* cmd namespace */

#endif
