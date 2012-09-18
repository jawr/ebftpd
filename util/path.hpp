#ifndef __UTIL_PATH_HPP
#define __UTIL_PATH_HPP

#include <string>

namespace util { namespace path
{

void TrimTrailingSlash(std::string& path, bool keepRootSlash = true);
std::string TrimTrailingSlashCopy(const std::string& path, bool keepRootSlash = true);
std::string Expand(const std::string& path);
std::string Join(const std::string& path1, const std::string& path2);
std::string Append(const std::string& path1, const std::string& path2);
std::string Dirname(std::string path);
std::string Basename(std::string path);
std::string Extension(const std::string& path);

} /* path namespace */
} /* util namespace */

#endif
