#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include "fs/filelock.hpp"
#include "util/error.hpp"

namespace fs
{

FileLock::FileLock(const std::string& path)
{
  fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) throw util::SystemError(errno);
  if (flock(fd, LOCK_EX) < 0)
  {
    int errno_ = errno;
    close(fd);
    fd = -1;
    throw util::SystemError(errno_);
  }
}

FileLock::~FileLock()
{
  if (fd >= 0)
  {
    flock(fd, LOCK_UN);
    close(fd);
  }
}

FileLockPtr FileLock::Create(const std::string& path)
{
  return FileLockPtr(new FileLock(path));
}

} /* fs namespace */


#ifdef FS_FILELOCK_TEST

#include <iostream>

int main()
{
  using namespace fs;
  
  {
    std::cout << "creating and releasing first lock" << std::endl;
    FileLockPtr lock1 = FileLock::Create("/tmp/somefile.txt");
  }
  
  {
    std::cout << "creating and releasing second lock" << std::endl;
    FileLockPtr lock1 = FileLock::Create("/tmp/somefile.txt");
  }
  
  std::cout << "creating and holding third lock" << std::endl;
  FileLockPtr lock1 = FileLock::Create("/tmp/somefile.txt");

  std::cout << "creating fourth lock, this should block forever" << std::endl;
  FileLockPtr lock2 = FileLock::Create("/tmp/somefile.txt"); 
}

#endif
