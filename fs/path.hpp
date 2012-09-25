#ifndef __FS_PATH_HPP
#define __FS_PATH_HPP

#include <string>
#include <ostream>
#include <istream>

namespace fs
{

class Path
{
protected:
  std::string path;
  
public:
  Path() { }
  Path(const std::string& path);
  Path(const char* path);
 
  Path& Join(const std::string& path);
  Path& Append(const std::string& path);
  
  Path Dirname() const;
  Path Basename() const;
  std::string Extension() const;
  Path Expand() const;
  
  Path& operator/=(const std::string& rhs) { return Join(rhs); }
  Path& operator+=(const std::string& rhs) { return Append(rhs); }

  Path& operator/=(const char* rhs) { return Join(rhs); }
  Path& operator+=(const char* rhs) { return Append(rhs); }

  bool operator<(const Path& rhs) const;
  bool operator>(const Path& rhs) const;
  
  operator std::string() { return path; }
  operator std::string() const { return path; }
  
  bool Absolute() const { return !path.empty() && path[0] == '/'; }
  bool Empty() const { return path.empty(); }
  
  const char* CString() const { return path.c_str(); }
  const std::string& ToString() const { return path; }
};

Path operator/(const Path& lhs, const std::string& rhs);
Path operator+(const Path& lhs, const std::string& rhs);
Path operator/(const Path& lhs, const char* rhs);
Path operator+(const Path& lhs, const char* rhs);
std::ostream& operator<<(std::ostream& os, const Path& path);
std::istream& operator>>(std::istream& is, Path& path);

Path PreparePath(const Path& siteRoot, const Path& workDir, const Path& path);

} /* fs namespace */

#endif
