#include <unistd.h>
#include <cerrno>
#include <sys/statvfs.h>
#include "fs/status.hpp"
#include "util/error.hpp"
#include "acl/path.hpp"
#include "ftp/client.hpp"

#include <iostream>

namespace fs
{

Status::Status() :
  client(nullptr),
  linkDirectory(false),
  linkRegularFile(false),
  statOkay(false)
{
}

Status::Status(const Path& path) :
  client(nullptr),
  path(RealPath(path)),
  linkDirectory(false),
  linkRegularFile(false),
  statOkay(false)
{
  Reset();
}

Status::Status(ftp::Client& client, const VirtualPath& path) :
  client(&client),
  path(MakeReal(path)),
  linkDirectory(false),
  linkRegularFile(false),
  statOkay(false)
{
  Reset();
}

Status& Status::Reset()
{
  if (path.IsEmpty()) throw std::logic_error("no path set");
  if (!statOkay)
  {
    if (client)
    {
      util::Error e = acl::path::FileAllowed<acl::path::View>(client->User(), MakeVirtual(path));
      if (!e) throw util::SystemError(e.Errno());
    }
    
    if (lstat(path.CString(), &native) < 0) throw util::SystemError(errno);
    
    if (client && IsDirectory())
    {
      util::Error e = acl::path::DirAllowed<acl::path::View>(client->User(), MakeVirtual(path));
      if (!e) throw util::SystemError(e.Errno());
    }
    
    if (IsSymLink())
    {
      struct stat st;
      if (!stat(path.CString(), &st) < 0) throw util::SystemError(errno);
      if (S_ISDIR(st.st_mode)) linkDirectory = true;
      else if (S_ISREG(st.st_mode)) linkRegularFile = true;
    }    
    statOkay = true;
  }
  return *this;
}

Status& Status::Reset(const Path& path)
{
  statOkay = false;
  this->path = RealPath(path);
  Reset();
  return *this;
}

Status& Status::Reset(ftp::Client& client, const VirtualPath& path)
{
  this->client = &client;
  statOkay = false;
  this->path = MakeReal(path);
  Reset();
  return *this;
}

bool Status::IsRegularFile() const
{
  return S_ISREG(native.st_mode) || linkRegularFile;
}

bool Status::IsDirectory() const
{
  return S_ISDIR(native.st_mode) || linkDirectory;
}

bool Status::IsSymLink() const
{
  return S_ISLNK(native.st_mode);
}

bool Status::IsExecutable() const
{
  return native.st_mode & S_IXOTH ||
         (geteuid() == native.st_uid && (native.st_mode & S_IXUSR)) ||
         (getegid() == native.st_gid && (native.st_mode & S_IXGRP));
}

bool Status::IsWriteable() const
{
  return native.st_mode & S_IWOTH ||
         (geteuid() == native.st_uid && (native.st_mode & S_IWUSR)) ||
         (getegid() == native.st_gid && (native.st_mode & S_IWGRP));
}

bool Status::IsReadable() const
{
  return native.st_mode & S_IROTH ||
         (geteuid() == native.st_uid && (native.st_mode & S_IRUSR)) ||
         (getegid() == native.st_gid && (native.st_mode & S_IRGRP));
}

off_t Status::Size() const
{
  return native.st_size;
}

const struct stat& Status::Native() const
{
  return native;
}

util::Error FreeDiskSpace(const Path& real, unsigned long long& freeBytes)
{
  struct statvfs sfs;

  if (statvfs(real.CString(), &sfs) <0)
    return util::Error::Failure(errno);

  freeBytes = sfs.f_bsize * sfs.f_bfree;
  return util::Error::Success();
}

} /* fs namespace */

#ifdef FS_STATUS_TEST

#include <iostream>

int main()
{
  {
    Status stat;
    
    std::cout << "isfile: " << stat.Reset("/home/bioboy").IsRegularFile() << std::endl;
    
    std::cout << "isdir: " << stat.IsDirectory() << std::endl;
    std::cout << "islink: " << stat.IsSymLink() << std::endl;
    std::cout << "size: " << stat.Size() << std::endl;
    std::cout << "isexec: " << stat.IsExecutable() << std::endl;
    std::cout << "isread: " << stat.IsReadable() << std::endl;
    std::cout << "iswrite: " << stat.IsWriteable() << std::endl;
  }
  
  try
  {
    Status stat("/home/notexist");
  }
  catch (util::SystemError& e)
  {
    if (e.ValidErrno()) std::cout << "errno: " << e.Errno() << " ";
    std::cout << e.what() << std::endl;
  }
  
  try
  {
    Status stat;
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

