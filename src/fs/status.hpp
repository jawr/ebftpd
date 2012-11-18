#ifndef __FS_STATUS_HPP
#define __FS_STATUS_HPP

#include <sys/stat.h>
#include "fs/path.hpp"
#include "util/error.hpp"

namespace ftp
{
class Client;
}

namespace fs
{

class Status
{
  ftp::Client* client;
  RealPath path;
  struct stat native;
  bool linkDirectory;
  bool linkRegularFile;
  bool statOkay;
  
  Status& Reset();
  
public:
  Status(const Path& path);
  Status(ftp::Client& client, const VirtualPath& path);
  Status();
  
  Status& Reset(const Path& path);
  Status& Reset(ftp::Client& client, const VirtualPath& path);  
  
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

util::Error FreeDiskSpace(const Path& real, unsigned long long& freeBytes);

} /* fs namespace */

#endif
