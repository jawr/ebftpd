#ifndef __FS_FILELOCK_HPP
#define __FS_FILELOCK_HPP

#include <memory>

namespace fs
{

class FileLock;

class FileLock
{
  int fd;

  FileLock(const std::string& path);
  
public:
  ~FileLock();

  static std::shared_ptr<FileLock> Create(const std::string& path);
};

} /* fs namespace */

#endif
