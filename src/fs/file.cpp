#include <cstdio>
#include <cerrno>
#include <sys/stat.h>
#include <fcntl.h>
#include "fs/file.hpp"
#include "acl/user.hpp"
#include "fs/path.hpp"
#include "ftp/client.hpp"
#include "fs/owner.hpp"
#include "util/misc.hpp"
#include "acl/path.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "util/randomstring.hpp"
#include "fs/status.hpp"
#include "util/error.hpp"
#include "logs/logs.hpp"

namespace PP = acl::path;

namespace fs
{

util::Error DeleteFile(const RealPath& path)
{
  if (unlink(path.CString()) < 0) return util::Error::Failure(errno);
  OwnerCache::Delete(path);
  return util::Error::Success();
}

util::Error DeleteFile(ftp::Client& client, const VirtualPath& path, off_t* size)
{
  util::Error e = PP::FileAllowed<PP::Delete>(client.User(), path);
  if (!e) return e;
  
  if (size)
  {
    try
    {
      *size = Status(MakeReal(path)).Size();
    }
    catch (const util::SystemError& e)
    {
      return util::Error::Failure(e.Errno());
    }
  }

  return DeleteFile(MakeReal(path));
}

util::Error RenameFile(const RealPath& oldPath, const RealPath& newPath)
{
  Owner owner = OwnerCache::Owner(oldPath);
  OwnerCache::Delete(oldPath);
  if (rename(oldPath.CString(), newPath.CString()) < 0) 
  {
    OwnerCache::Chown(oldPath, owner);
    return util::Error::Failure(errno);
  }
  OwnerCache::Chown(newPath, owner);
  return util::Error::Success();
}

util::Error RenameFile(ftp::Client& client, const VirtualPath& oldPath,
                 const VirtualPath& newPath)                 
{
  util::Error e = PP::FileAllowed<PP::Rename>(client.User(), oldPath);
  if (!e) return e;

  e = PP::FileAllowed<PP::Upload>(client.User(), newPath);
  if (!e) return e;
  
  return RenameFile(MakeReal(oldPath), MakeReal(newPath));
}

FileSinkPtr CreateFile(ftp::Client& client, const VirtualPath& path)
{
  util::Error e(PP::FileAllowed<PP::Upload>(client.User(), path));
  if (!e) throw util::SystemError(e.Errno());
  
  unsigned long long freeBytes;
  e = fs::FreeDiskSpace(MakeReal(path).Dirname(), freeBytes);
  if (!e) throw util::SystemError(e.Errno());
  
  if (cfg::Get().FreeSpace() > freeBytes / 1024 / 1024)
    throw util::SystemError(ENOSPC);

  mode_t mode = cfg::Get().DlIncomplete() ? 0755 : 0644;
    
  int fd = open(MakeReal(path).CString(), O_CREAT | O_WRONLY | O_EXCL, mode);
  if (fd < 0)
  {
    if (errno != EEXIST) throw util::SystemError(errno);

    e = PP::FileAllowed<PP::Overwrite>(client.User(), path);
    if (!e) throw util::SystemError(EEXIST);
    
    fd = open(MakeReal(path).CString(), O_WRONLY | O_TRUNC);
    if (fd < 0) throw util::SystemError(errno);
  }

  OwnerCache::Chown(MakeReal(path), Owner(client.User().UID(), 
      client.User().PrimaryGID()));

  return FileSinkPtr(new FileSink(fd, boost::iostreams::close_handle));
}

FileSinkPtr AppendFile(ftp::Client& client, const VirtualPath& path, off_t offset)
{
  util::Error e = PP::FileAllowed<PP::Resume>(client.User(), path);
  if (!e) throw util::SystemError(e.Errno());

  unsigned long long freeBytes;
  e = fs::FreeDiskSpace(MakeReal(path).Dirname(), freeBytes);
  if (!e) throw util::SystemError(e.Errno());
  
  if (cfg::Get().FreeSpace() > freeBytes / 1024 / 1024)
    throw util::SystemError(ENOSPC);

  int fd = open(MakeReal(path).CString(), O_WRONLY | O_APPEND);
  if (fd < 0) throw util::SystemError(errno);
 
 FileSinkPtr fout(new FileSink(fd, boost::iostreams::close_handle));

  try
  {
    std::streampos size = fout->seek(0, std::ios_base::end);
    if (offset < size && ftruncate(fout->handle(), offset) < 0)
      throw util::SystemError(errno);
    fout->seek(0, std::ios_base::end);  
  }
  catch (const std::ios_base::failure& e)
  {
    throw util::SystemError(errno);
  }
  
  return fout;
}

FileSourcePtr OpenFile(ftp::Client& client, const VirtualPath& path)
{
  util::Error e = PP::FileAllowed<PP::Download>(client.User(), path);
  if (!e) throw util::SystemError(e.Errno());

  int fd = open(MakeReal(path).CString(), O_RDONLY);
  if (fd < 0) throw util::SystemError(errno);
  return FileSourcePtr(new FileSource(fd, boost::iostreams::close_handle));
}

util::Error UniqueFile(ftp::Client& client, const VirtualPath& path, 
                       size_t filenameLength, VirtualPath& uniquePath)
{ 
  util::Error e = PP::FileAllowed<PP::Upload>(client.User(), path / "dummyfile");
  if (!e) throw util::SystemError(e.Errno());

  for (int i = 0; i < 1000; ++i)
  {
    std::string filename = util::RandomString(filenameLength, 
            util::RandomString::alphaNumeric);
    
    uniquePath = path / filename;
    try
    {
      Status status(MakeReal(uniquePath));
    }
    catch (const util::SystemError& e)
    {
      if (e.Errno() == ENOENT) return util::Error::Success();
      else return util::Error::Failure(e.Errno());
    }
  }
  
  return util::Error::Failure();
}

bool IsIncomplete(const RealPath& path)
{
  static const time_t maxInactivity = 30;
  
  try
  {
    Status status(path);
    if (status.IsExecutable() && 
        time(NULL) - status.Native().st_mtime < maxInactivity)
      return true;
  }
  catch (const util::SystemError&)
  {
  }
  
  return false;
}

} /* fs namespace */

#ifdef FS_FILE_TEST

#include <iostream>

int main()
{
  using namespace fs;

  util::Error e = DeleteFile("somefile");
  std::cout << "delete: " << (e ? "true" : "false") << " " << e.Message() << " " << e.Errno() << std::endl;
  
  e = RenameFile("one", "two");
  std::cout << "rename: " << (e ? "true" : "false") << " " << e.Message() << " " << e.Errno() << std::endl;
  
  try
  {
    OutStreamPtr os(CreateFile("newfile"));
    *os << "test" << std::endl;
  }
  catch (const util::SystemError& e)
  {
    std::cout << "create file: " << e.Message() << std::endl;
  }
  
  try
  {
    OutStreamPtr os(AppendFile("newfile"));
    
    *os << "test2" << std::endl;
  }
  catch (const util::SystemError& e)
  {
    std::cout << "append file: " << e.Message() << std::endl;
  }
  
  try
  {
    OutStreamPtr os(AppendFile("notexist"));
  }
  catch (const util::SystemError& e)
  {
    std::cout << "append: " << e.Message() << " " << e.Errno() << std::endl;
  }
  
  InStreamPtr is(OpenFile("newfile"));
  
  std::string line;
  while (std::getline(*is, line)) std::cout << line << std::endl;
}

#endif
