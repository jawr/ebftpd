#ifndef __UTIL_PATH_HPP
#define __UTIL_PATH_HPP

#include <string>
#include <vector>

namespace util { namespace path
{

void TrimTrailingSlash(std::string& path, bool keepRootSlash = true);
std::string TrimTrailingSlashCopy(const std::string& path, bool keepRootSlash = true);
std::string Resolve(const std::string& path);
std::string Join(const std::string& path1, const std::string& path2);
std::string Append(const std::string& path1, const std::string& path2);
std::string Dirname(std::string path);
std::string Basename(std::string path);
std::string Extension(const std::string& path);
std::string NoExtension(const std::string& path);
std::string Relative(const std::string& workPath, const std::string& path);
std::vector<std::string> Glob(const std::string& path, bool tilde = true);
bool WildcardMatch(const std::string& pattern, const std::string& path, bool iCase = false);
bool Readlink(const std::string& source, std::string& dest);
bool Realpath(const std::string& source, std::string& dest);

} /* path namespace */
} /* util namespace */

#endif
