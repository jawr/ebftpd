#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "fs/file.hpp"
#include "acl/user.hpp"
#include "fs/path.hpp"
#include "ftp/client.hpp"
#include "fs/owner.hpp"
#include "util/misc.hpp"
#include "acl/check.hpp"

extern const fs::Path dummySiteRoot;

namespace PP = acl::PathPermission;

namespace fs
{

util::Error DeleteFile(ftp::Client& client, const Path& path)
{
  Path absolute = (client.WorkDir() / path).Expand();
  util::Error e = PP::FileAllowed<PP::Delete>(client.User(), absolute);
  if (!e) return e;
  Path real = dummySiteRoot + absolute;
  if (unlink(real.CString()) < 0) return util::Error::Failure(errno);
  OwnerCache::Delete(real);
  return util::Error::Success();
}

util::Error RenameFile(ftp::Client& client, const Path& oldPath,
                 const Path& newPath)
{
  Path oldAbsolute = (client.WorkDir() / oldPath).Expand();
  util::Error e(PP::FileAllowed<PP::Rename>(client.User(), oldAbsolute));
  if (!e) return e;

  Path newAbsolute = (client.WorkDir() / newPath).Expand();
  e = PP::FileAllowed<PP::Upload>(client.User(), newAbsolute);
  if (!e) return e;

  Path oldReal = dummySiteRoot + oldAbsolute;
  Path newReal = dummySiteRoot + newAbsolute;

  Owner owner = OwnerCache::Owner(oldReal);
  if (rename(oldReal.CString(), newReal.CString()) < 0) return util::Error::Failure(errno);
  OwnerCache::Chown(newReal, owner);
  OwnerCache::Delete(oldReal);
  return util::Error::Success();
}

OutStreamPtr CreateFile(ftp::Client& client, const Path& path)
{
  Path absolute = (client.WorkDir() / path).Expand();
  util::Error e(PP::FileAllowed<PP::Upload>(client.User(), absolute));
  if (!e) throw util::SystemError(e.Errno());

  Path real = dummySiteRoot + absolute;
  int fd = open(real.CString(), O_CREAT | O_WRONLY | O_EXCL, 0777);
  if (fd < 0) throw util::SystemError(errno);

  OwnerCache::Chown(real, Owner(client.User().UID(), client.User().PrimaryGID()));

  return OutStreamPtr(new OutStream(fd, boost::iostreams::close_handle));
}

OutStreamPtr AppendFile(ftp::Client& client, const Path& path)
{
  Path absolute = (client.WorkDir() / path).Expand();
  util::Error e = PP::FileAllowed<PP::Resume>(client.User(), absolute);
  if (!e) throw util::SystemError(e.Errno());

  Path real = dummySiteRoot + absolute;
  int fd = open(real.CString(), O_WRONLY | O_APPEND);
  if (fd < 0) throw util::SystemError(errno);
  return OutStreamPtr(new OutStream(fd, boost::iostreams::close_handle));
}

InStreamPtr OpenFile(ftp::Client& client, const Path& path)
{
  Path absolute = (client.WorkDir() / path).Expand();
  util::Error e = PP::FileAllowed<PP::Download>(client.User(), absolute);
  if (!e) throw util::SystemError(e.Errno());

  Path real = dummySiteRoot + absolute;
  int fd = open(real.CString(), O_RDONLY);
  if (fd < 0) throw util::SystemError(errno);
  return InStreamPtr(new InStream(fd, boost::iostreams::close_handle));
}

util::Error UniqueFile(ftp::Client& client, const Path& path, 
                       size_t filenameLength, Path& uniquePath)
{ 
  Path absolute = (client.WorkDir() / path).Expand();
  
  util::Error e = PP::FileAllowed<PP::Upload>(client.User(), absolute / "dummyfile");
  if (!e) throw util::SystemError(e.Errno());

  Path real = dummySiteRoot + absolute;

  for (int i = 0; i < 1000; ++i)
  {
    std::string filename = util::RandomString(filenameLength);
    Path realUniquePath = real / filename;
    try
    {
      Status status(realUniquePath);
    }
    catch (const util::SystemError& e)
    {
      if (e.Errno() == ENOENT)
      {
        uniquePath = absolute / filename;
        return util::Error::Success();
      }
      else return util::Error::Failure(e.Errno());
    }
  }
  
  return util::Error::Failure();
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
