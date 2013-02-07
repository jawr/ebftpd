#ifndef __FS_STATUS_HPP
#define __FS_STATUS_HPP

#include <string>
#include <sys/stat.h>

namespace util
{
class Error;
}

namespace util { namespace path
{

class Status
{
  std::string path;
  struct stat native;
  bool linkDirectory;
  bool linkRegularFile;
  bool statOkay;
  
  Status& Reset();
  
public:
  Status();
  Status(const std::string& path);
  
  Status& Reset(const std::string& path);
  
  bool IsRegularFile() const;
  bool IsDirectory() const;
  bool IsSymLink() const;
  
  bool IsExecutable() const;
  bool IsReadable() const;
  bool IsWriteable() const;
  uid_t UID() const { return native.st_uid; }
  gid_t GID() const { return native.st_gid; }
  
  off_t Size() const;
  
  const struct stat& Native() const;
};

util::Error FreeDiskSpace(const std::string& real, unsigned long long& freeBytes);

} /* path namespace */
} /* fs namespace */

#endif
