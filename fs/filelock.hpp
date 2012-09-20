#ifndef __FS_FILELOCK_HPP
#define __FS_FILELOCK_HPP

#include <tr1/memory>

namespace fs
{

class FileLock;
typedef std::tr1::shared_ptr<FileLock> FileLockPtr;

class FileLock
{
  int fd;

  FileLock(const std::string& path);
  
public:
  ~FileLock();

  static FileLockPtr Create(const std::string& path);
};

} /* fs namespace */

#endif
