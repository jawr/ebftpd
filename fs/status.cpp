#include <unistd.h>
#include <cerrno>
#include "status.hpp"
#include "exception.hpp"

namespace fs
{

Status::Status(const std::string& path) :
  path(path),
  statOkay(false)
{
  Check();
}

Status& Status::Check()
{
  if (path.empty()) throw std::logic_error("no path set");
  if (!statOkay)
  {
    if (stat(path.c_str(), &native) < 0) throw FileSystemError(errno);
    statOkay = true;
  }
  return *this;
}

Status& Status::Check(const std::string& path)
{
  statOkay = false;
  this->path = path;
  Check();
  return *this;
}

bool Status::IsRegularFile()
{
  Check();
  return S_ISREG(native.st_mode);
}

bool Status::IsDirectory()
{
  Check();
  return S_ISDIR(native.st_mode);
}

bool Status::IsLink()
{
  Check();
  return S_ISLNK(native.st_mode);
}

off_t Status::Size()
{
  Check();
  return native.st_size;
}

const struct stat& Status::Native()
{
  Check();
  return native;
}

} /* fs namespace */

#ifdef FS_STATUS_TEST

#include <iostream>

int main()
{
  {
    fs::Status stat;
    
    std::cout << stat.Check("/home/bioboy").IsRegularFile() << std::endl;
    
    std::cout << stat.IsDirectory() << std::endl;
    std::cout << stat.IsLink() << std::endl;
    std::cout << sizeof(off_t) << " " << sizeof(size_t) << std::endl;
    std::cout << stat.Size() << std::endl;
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
    // assigned using Check()
    stat.Size();
  }
  catch (const std::logic_error& e)
  {
    std::cout << "class used incorrectly: " << e.what() << std::endl;
  }
}

#endif

