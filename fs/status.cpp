#include <unistd.h>
#include <cerrno>
#include "status.hpp"
#include "error.hpp"

namespace fs
{

Status::Status(const std::string& path) :
  path(path),
  statOkay(false)
{
  Reset();
}

Status& Status::Reset()
{
  if (path.empty()) throw std::logic_error("no path set");
  if (!statOkay)
  {
    if (stat(path.c_str(), &native) < 0) throw FileSystemError(errno);
    statOkay = true;
  }
  return *this;
}

Status& Status::Reset(const std::string& path)
{
  statOkay = false;
  this->path = path;
  Reset();
  return *this;
}

bool Status::IsRegularFile()
{
  Reset();
  return S_ISREG(native.st_mode);
}

bool Status::IsDirectory()
{
  Reset();
  return S_ISDIR(native.st_mode);
}

bool Status::IsLink()
{
  Reset();
  return S_ISLNK(native.st_mode);
}

bool Status::IsExecutable()
{
  Reset();
  return native.st_mode & S_IXOTH ||
         (geteuid() == native.st_uid && (native.st_mode & S_IXUSR)) ||
         (getegid() == native.st_gid && (native.st_mode & S_IXGRP));
}

bool Status::IsWriteable()
{
  Reset();
  return native.st_mode & S_IWOTH ||
         (geteuid() == native.st_uid && (native.st_mode & S_IWUSR)) ||
         (getegid() == native.st_gid && (native.st_mode & S_IWGRP));
}

bool Status::IsReadable()
{
  Reset();
  return native.st_mode & S_IROTH ||
         (geteuid() == native.st_uid && (native.st_mode & S_IRUSR)) ||
         (getegid() == native.st_gid && (native.st_mode & S_IRGRP));
}

const std::string& Status::Path()
{
  Reset();
  return path;
}

off_t Status::Size()
{
  Reset();
  return native.st_size;
}

const struct stat& Status::Native()
{
  Reset();
  return native;
}

} /* fs namespace */

#ifdef FS_STATUS_TEST

#include <iostream>

int main()
{
  {
    fs::Status stat;
    
    std::cout << "isfile: " << stat.Reset("/home/bioboy").IsRegularFile() << std::endl;
    
    std::cout << "isdir: " << stat.IsDirectory() << std::endl;
    std::cout << "islink: " << stat.IsLink() << std::endl;
    std::cout << "size: " << stat.Size() << std::endl;
    std::cout << "isexec: " << stat.IsExecutable() << std::endl;
    std::cout << "isread: " << stat.IsReadable() << std::endl;
    std::cout << "iswrite: " << stat.IsWriteable() << std::endl;
  }
  
  try
  {
    fs::Status stat("/home/notexist");
  }
  catch (fs::FileSystemError& e)
  {
    if (e.ValidErrno()) std::cout << "errno: " << e.Errno() << " ";
    std::cout << e.what() << std::endl;
  }
  
  try
  {
    fs::Status stat;
    // path hasn't been passed to constructor or
    // assigned using Reset()
    stat.Size();
  }
  catch (const std::logic_error& e)
  {
    std::cout << "class used incorrectly: " << e.what() << std::endl;
  }
}

#endif

