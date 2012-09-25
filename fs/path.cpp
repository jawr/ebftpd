#include "fs/path.hpp"
#include "util/path.hpp"

namespace fs
{

Path::Path(const std::string& path) :
  path(path)
{
}

Path::Path(const char* path) :
  path(path)
{
}

Path& Path::Join(const std::string& path)
{
  this->path = util::path::Join(this->path, path);
  return *this;
}

Path& Path::Append(const std::string& path)
{
  this->path = util::path::Append(this->path, path);
  return *this;
}

Path Path::Dirname() const
{
  return util::path::Dirname(path);
}

Path Path::Basename() const
{
  return util::path::Basename(path);
}

std::string Path::Extension() const
{
  return util::path::Extension(path);
}

Path Path::Expand() const
{
  return util::path::Expand(path);
}

Path operator/(const Path& lhs, const std::string& rhs)
{
  return Path(lhs) /= rhs;
}

Path operator+(const Path& lhs, const std::string& rhs)
{
  return Path(lhs) += rhs;
}

Path operator/(const Path& lhs, const char* rhs)
{
  return Path(lhs) /= rhs;
}

Path operator+(const Path& lhs, const char* rhs)
{
  return Path(lhs) += rhs;
}

std::ostream& operator<<(std::ostream& os, const Path& path)
{
  return (os << std::string(path));
}

std::istream& operator>>(std::istream& is, Path& path)
{
  std::string temp;
  is >> temp;
  path = temp;
  return is;
}

Path PreparePath(const Path& siteRoot, const Path& workDir, const Path& path)
{
  Path result(siteRoot);
  if (!path.Absolute()) result /= workDir;
  return result /= path;
}

} /* fs namespace */

#ifdef FS_PATH_TEST

#include <iostream>

int main()
{
  using namespace fs;
  
  /*Path path("test");
	std::cout << path << std::endl;
  std::cout << path.CString() << std::endl;
  std::cout << Path("wow").Join("wow2") << std::endl;
  Path p = Path("/one") / "two" / "three";
  std::cout << "joined: " << p << std::endl;
  std::cout << "dirname: " << p.Dirname() << std::endl;
  std::cout << "basename: " << p.Basename() << std::endl;
  std::cout << "extension: " << p.Extension() << std::endl;
  std::cout << "extension: " << Path("file.txt") << std::endl;
  std::cout << p << std::endl;
  p /= "..";
  std::cout << "..: " << p << std::endl;
  p /= ".";
  std::cout << ".: " << p << std::endl;
  p /= "somedir/../someother/dir";
  std::cout << "joined: " << p << std::endl;
  std::cout << "..: " << (p / "..") << std::endl;
  
  std::cout << PreparePath("/home/bioboy/ftpd/site", "/incoming/mp3/", "/incoming/iso") << std::endl;
  std::cout << PreparePath("/home/bioboy/ftpd/site", "/incoming/mp3", "0909") << std::endl;*/
  
 // MasterPath path ("pow");
//  std::cout << path << std::endl;
}

#endif
