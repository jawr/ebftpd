#ifndef __FS_UTILITY_HPP
#define __FS_UTILITY_HPP

#include <string>

namespace fs
{

bool IsFile(const std::string& path);
bool IsDirectory(const std::string& path);
bool IsLink(const std::string& path);

} /* fs namespace */

#endif
