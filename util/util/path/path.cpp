//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <limits.h>
#include <glob.h>
#include <fnmatch.h>
#include <stdexcept>
#include <unistd.h>
#include "util/string.hpp"
#include "util/path/path.hpp"

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

void SplitSegments(const std::string& path, std::vector<std::string>& segments)
{
  segments.clear();
  std::string::size_type pos1 = 0;
  std::string::size_type pos2 = 0;
  size_t total = 1;

  while ((pos2 = path.find('/', pos1)) != std::string::npos)
  {
    ++total;
    pos1 = pos2 + 1;
  }
  
  segments.reserve(total);

  pos1 = 0;
  while ((pos2 = path.find('/', pos1)) != std::string::npos)
  {
    if (pos2 != pos1 || pos1 == 0) 
      segments.emplace_back(path.substr(pos1, pos2 - pos1));
    pos1 = pos2 + 1;
  }

  if (pos2 != path.length() - 1) segments.emplace_back(path.substr(pos1));
}

std::string JoinSegments(const std::vector<std::string>& segments)
{
  if (segments.empty()) return std::string();
  
  std::string::size_type len = 0;
  std::vector<std::string>::size_type size = segments.size();
  for (std::vector<std::string>::size_type i = 0; i < size; ++i)
    len += segments[i].length() + 1;
  
  std::string result(segments[0]);
  result.reserve(len);
  for (std::vector<std::string>::size_type i = 1; i < size; ++i)
  {
    result += '/';
    result += segments[i];
  }

  return result;
}

std::string Resolve(const std::string& path)
{
  if (path.empty()) return std::string();
  bool absolute = path[0] == '/';
  
  std::vector<std::string> segments;
  SplitSegments(path, segments);
  for (auto it = segments.begin(); it != segments.end();)
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
  
  std::string result(JoinSegments(segments));
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
  if (result.empty()) result = "/";
  return result;
}

std::string Dirname(std::string path)
{
  TrimTrailingSlash(path);
  if (path == "/") return path;
  std::string::size_type pos = path.find_last_of('/');
  if (pos == 0) return "/";
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

std::string NoExtension(const std::string& path)
{
  std::string basename(Basename(path));
  return basename.substr(0, basename.find_last_of('.'));
}

std::string Relative(const std::string& workPath, const std::string& path)
{
   if (path.empty()) return std::string();
  if (path[0] != '/') return path;
  
  if (workPath.empty() || workPath[0] != '/') 
    throw std::logic_error("workPath must be absolute work directory");

  if (workPath == "/") return path.substr(1);
    
  std::vector<std::string> wpSegments;
  SplitSegments(workPath, wpSegments);
  
  std::vector<std::string> pSegments;
  SplitSegments(path, pSegments);
  
  std::vector<std::string> result;
  
  size_t maxLen = std::min(wpSegments.size(), pSegments.size() - 1);
  size_t i = 0;
  for (; i < maxLen; ++i)
    if (wpSegments[i] != pSegments[i]) break;
    
  for (size_t j = i; j < wpSegments.size(); ++j)
    result.emplace_back("..");
    
  result.insert(result.end(), pSegments.begin() + i, pSegments.end());
  return JoinSegments(result);
}

std::vector<std::string> Glob(const std::string& path, bool tilde)
{
  glob_t glMatches;
  std::vector<std::string> matches;
  if (!glob(path.c_str(), tilde ? GLOB_TILDE : 0, nullptr, &glMatches))
  {
    matches.reserve(glMatches.gl_pathc);
    for (size_t i = 0; i < glMatches.gl_pathc; ++i)
    {
      matches.push_back(glMatches.gl_pathv[i]);
    }
    globfree(&glMatches);
  }
  return matches;
}

bool WildcardMatch(const std::string& pattern, const std::string& path, bool iCase)
{
  int flags = iCase ? FNM_CASEFOLD : 0;
  flags |= FNM_PATHNAME;
  return !fnmatch(pattern.c_str(), path.c_str(), flags);
}

bool Readlink(const std::string& source, std::string& dest)
{
  char buf[PATH_MAX];
  ssize_t len = readlink(source.c_str(), buf, sizeof(buf));
  if (len < 0) return false;
  dest.assign(buf, len);
  return true;
}

bool Realpath(const std::string& source, std::string& dest)
{
  char buf[PATH_MAX];
  if (!realpath(source.c_str(), buf)) return false;
  dest.assign(buf);
  return true;
}

} /* path namespace */
} /* util namespace */
