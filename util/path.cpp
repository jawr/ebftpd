#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "path.hpp"

namespace util { namespace path
{

void TrimTrailingSlash(std::string& path, bool keepRootSlash)
{
  std::string::size_type pos = path.length() - 1;
  if (path[pos] != '/') return;
  if (keepRootSlash && path.length() == 1) return;
  path.erase(pos);
}

std::string TrimTrailingSlashCopy(const std::string& path, bool keepRootSlash)
{
  std::string result(path);
  TrimTrailingSlash(result, keepRootSlash);
  return result;
}

std::string Expand(const std::string& path)
{
  if (path.empty()) return "";
  bool absolute = path[0] == '/';
  
  std::vector<std::string> segments;
  boost::split(segments, path, boost::is_any_of("/"), boost::token_compress_on);
  for (std::vector<std::string>::iterator it =
       segments.begin(); it != segments.end();)
  {
    if (*it == ".") it = segments.erase(it);
    else if (*it == "..")
    {
      if (it == segments.begin()) segments.erase(it);
      else it = segments.erase(it - 1, it + 1);
    }
    else
      ++it;
  }
  
  std::string result = boost::join(segments, "/");
  if (result.empty()) return "/";
  if (absolute && result[0] != '/') result.insert(0, 1, '/');
  TrimTrailingSlash(result);
  return result;
}

std::string Join(const std::string& path1, const std::string& path2)
{
  if (path1.empty()) return path2;
  else if (path2.empty()) return path1;
  if (path2[0] == '/') return path2;
  std::string result(path1);
  if (result[result.length()-1] != '/') result += '/';
  result += path2;
  return result;
}

std::string Append(const std::string& path1, const std::string& path2)
{
  if (path1.empty()) return path2;
  else if (path2.empty()) return path1;
  
  std::string::size_type len1 = path1.length();
  if (path1[len1 - 1] == '/') --len1;
  
  std::string::size_type len2 = path2.length();
  if (path2[len2 - 1] == '/') --len2;
  
  std::string result(path1.substr(0, len1));
  if (path2[0] != '/') result += '/';
  result += path2.substr(0, len2);
  return result;
}

std::string Dirname(std::string path)
{
  TrimTrailingSlash(path);
  if (path == "/") return path;
  std::string::size_type pos = path.find_last_of('/');
  if (pos == std::string::npos) return "";
  return path.substr(0, pos);
}

std::string Basename(std::string path)
{
  TrimTrailingSlash(path);
  if (path == "/") return path;
  std::string::size_type pos = path.find_last_of('/');
  if (pos == std::string::npos) return path;
  return path.substr(pos + 1);
}

std::string Extension(const std::string& path)
{
  std::string::size_type pos = path.find_last_of('.');
  if (pos == std::string::npos) return "";
  return path.substr(pos + 1);
}

} /* path namespace */
} /* util namespace */

#ifdef UTIL_PATH_TEST

#include <iostream>

int main()
{
  using namespace util::path;
  
  std::cout << "trim trailing slash: " << TrimTrailingSlashCopy("/has/trailing/slash/") << std::endl;
  std::cout << "trim trailing slash: " << TrimTrailingSlashCopy("/no/trialing/slash") << std::endl;
  std::cout << "trim trailing slash, keep root: " << TrimTrailingSlashCopy("/", true) << std::endl;
  std::cout << "join: " << Join("/first/path", "second/path") << std::endl;
  std::cout << "join: " << Join("/first/path", "/second/path") << std::endl;
  std::cout << "append: " << Append("/first/path", "second/path") << std::endl;
  std::cout << "append: " << Append("/first/path", "/second/path") << std::endl;
  std::cout << "dirname: " << Dirname("/") << std::endl;
  std::cout << "dirname: " << Dirname("/this/is/a/path") << std::endl;
  std::cout << "dirname: " << Dirname("this_is_also_a_path") << std::endl;
  std::cout << "basename: " << Basename("/this/is/a/path") << std::endl;
  std::cout << "basename, trailing slash: " << Basename("/this/is/a/path/") << std::endl;
  std::cout << "basename: " << Basename("another_path") << std::endl;
  std::cout << "extension: " << Extension("/some/path/file.ext") << std::endl;
  std::cout << "extension: " << Extension("/some/path/file_no_exit") << std::endl;
  std::cout << "expand: " << Expand("/test/./two/three/four/../../wow") << std::endl;
  std::cout << "expand: " << Expand("/..") << std::endl;
  std::cout << "expand: " << Expand("/../somewhere///multiple//slashes///") << std::endl;
}

#endif
