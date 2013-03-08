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

std::shared_ptr<FileLock> FileLock::Create(const std::string& path)
{
  return std::shared_ptr<FileLock>(new FileLock(path));
}

} /* fs namespace */
