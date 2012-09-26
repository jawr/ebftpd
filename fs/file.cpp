#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "fs/file.hpp"
#include "acl/user.hpp"
#include "fs/path.hpp"
#include "ftp/client.hpp"

namespace fs
{

namespace
{
const std::string dummySiteRoot = "/home/bioboy/ftpd/site";

}

util::Error DeleteFile(const Path& path)
{
  Path real = dummySiteRoot + path;
  if (unlink(real.CString()) < 0) return util::Error::Failure(errno);
  else return util::Error::Success();
}

util::Error DeleteFile(ftp::Client& client, const Path& path)
{
  Path absolute = (client.WorkDir() / path).Expand();
  // check ACLs
  return DeleteFile(absolute);
}

// implement alternative rename for when not on same filesystem?
// using copy / delete instead?
util::Error RenameFile(const Path& oldPath, const Path& newPath)
{
  Path oldReal = dummySiteRoot + oldPath;
  Path newReal = dummySiteRoot + newPath;

  if (rename(oldReal.CString(), newReal.CString()) < 0) return util::Error::Failure(errno);
  else return util::Error::Success();
}

util::Error RenameFile(ftp::Client& client, const Path& oldPath,
                 const Path& newPath)
{
  Path oldAbsolute = (client.WorkDir() / oldPath).Expand();
  Path newAbsolute = (client.WorkDir() / newPath).Expand();
  // check ACLs
  return RenameFile(oldAbsolute, newAbsolute);
}

OutStreamPtr CreateFile(const Path& path)
{
  Path real = dummySiteRoot + path;
  int fd = open(real.CString(), O_CREAT | O_WRONLY | O_EXCL, 0777);
  if (fd < 0) throw util::SystemError(errno);
  return OutStreamPtr(new OutStream(fd, boost::iostreams::close_handle));
}

OutStreamPtr CreateFile(ftp::Client& client, const Path& path)
{
  Path absolute = (client.WorkDir() / path).Expand();
  // check ACLs
  OutStreamPtr os(CreateFile(absolute));
  // update owner file
  return os;
}

OutStreamPtr AppendFile(const Path& path)
{
  Path real = dummySiteRoot + path;
  int fd = open(real.CString(), O_WRONLY | O_APPEND);
  if (fd < 0) throw util::SystemError(errno);
  return OutStreamPtr(new OutStream(fd, boost::iostreams::close_handle));
}

OutStreamPtr AppendFile(ftp::Client& client, const Path& path)
{
  Path absolute = (client.WorkDir() / path).Expand();
  // check ACLs
  return AppendFile(absolute);
}

InStreamPtr OpenFile(const Path& path)
{
  Path real = dummySiteRoot + path;
  int fd = open(real.CString(), O_RDONLY);
  if (fd < 0) throw util::SystemError(errno);
  return InStreamPtr(new InStream(fd, boost::iostreams::close_handle));
}

InStreamPtr OpenFile(ftp::Client& client, const Path& path)
{
  Path absolute = (client.WorkDir() / path).Expand();
  // check ACLs
  return OpenFile(absolute);
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
