#ifndef __FS_STATUS_HPP
#define __FS_STATUS_HPP

#include <sys/stat.h>
#include "fs/path.hpp"

namespace fs
{

class Status
{
  fs::Path path;
  struct stat native;
  bool statOkay;
  
  Status& Reset();
  
public:
  Status(const fs::Path& path);
  Status();
  
  Status& Reset(const fs::Path& path);
  
  bool IsRegularFile();
  bool IsDirectory();
  bool IsLink();
  
  bool IsExecutable();
  bool IsReadable();
  bool IsWriteable();
  
  const fs::Path& Path();
  off_t Size();
  
  const struct stat& Native();
};

} /* fs namespace */

#endif
