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
  bool linkDirectory;
  bool linkRegularFile;
  bool statOkay;
  
  Status& Reset();
  
public:
  Status(const fs::Path& path);
  Status();
  
  Status& Reset(const fs::Path& path);
  
  bool IsRegularFile() const;
  bool IsDirectory() const;
  bool IsSymLink() const;
  
  bool IsExecutable() const;
  bool IsReadable() const;
  bool IsWriteable() const;
  uid_t UID() const { return native.st_uid; }
  gid_t GID() const { return native.st_gid; }
  
  const fs::Path& Path() const;
  off_t Size() const;
  
  const struct stat& Native() const;
};

} /* fs namespace */

#endif
