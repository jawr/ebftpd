#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "fs/file.hpp"
#include "acl/user.hpp"

namespace fs
{

Error DeleteFile(const std::string& path)
{
  if (unlink(path.c_str()) < 0) return Error::Failure(errno);
  else return Error::Success();
}

Error DeleteFile(const acl::User& user, std::string& path)
{
  // check ACLs
  return DeleteFile(path);
}

Error DeleteFile(const ftp::Client& client, const std::string& path)
{
  assert(false && "not implemnted");
}

// implement alternative rename for when not on same filesystem?
// using copy / delete instead?
Error RenameFile(const std::string& oldPath, const std::string& newPath)
{
  if (rename(oldPath.c_str(), newPath.c_str()) < 0) return Error::Failure(errno);
  else return Error::Success();
}

Error RenameFile(const acl::User& user, const std::string& oldPath,
                 const std::string& newPath)
{
  // check ACLs
  return RenameFile(oldPath, newPath);
}

Error RenameFile(const ftp::Client& client, const std::string& oldPath,
                 const std::string& newPath)
{
  assert(false && "not implemented");
}

OutStreamPtr CreateFile(const std::string& path)
{
  using namespace boost::iostreams;
  
  int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_EXCL, 0777);
  if (fd < 0) throw FileSystemError(errno);
  
  return OutStreamPtr(new OutStream(fd, boost::iostreams::close_handle));
}

OutStreamPtr CreateFile(const acl::User& user, const std::string& path)
{
  // check ACLs
  OutStreamPtr os(CreateFile(path));
  // update owner file
  return os;
}

OutStreamPtr CreateFile(const ftp::Client& client, const std::string& path)
{
  assert(false && "not implemented");
}

OutStreamPtr AppendFile(const std::string& path)
{
  using namespace boost::iostreams;
  
  int fd = open(path.c_str(), O_WRONLY | O_APPEND);
  if (fd < 0) throw FileSystemError(errno);
  
  return OutStreamPtr(new OutStream(fd, boost::iostreams::close_handle));
}

OutStreamPtr AppendFile(const acl::User& user, const std::string& path)
{
  // check ACLs
  return AppendFile(path);
}

OutStreamPtr AppendFile(const ftp::Client& client, const std::string& path)
{
  assert(false && "not implemented");
}

InStreamPtr OpenFile(const std::string& path)
{
  int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) throw FileSystemError(errno);
  
  return InStreamPtr(new InStream(fd, boost::iostreams::close_handle));
}

InStreamPtr OpenFile(const acl::User& user, const std::string& path)
{
  // check ACLs
  return OpenFile(path);
}

InStreamPtr OpenFile(const ftp::Client& client, const std::string& path)
{
  assert(false && "not implemented");
}

} /* fs namespace */

#ifdef FS_FILE_TEST

#include <iostream>

int main()
{
  using namespace fs;

  Error e = DeleteFile("/tmp/somefile");
  std::cout << "delete: " << (e ? "true" : "false") << " " << e.Message() << " " << e.Errno() << std::endl;
  
  e = RenameFile("/tmp/one", "/tmp/two");
  std::cout << "rename: " << (e ? "true" : "false") << " " << e.Message() << " " << e.Errno() << std::endl;
  
  OutStreamPtr os(CreateFile("/tmp/newfile"));
  
  *os << "test" << std::endl;
  
  os = AppendFile("/tmp/newfile");
  
  *os << "test2" << std::endl;
  
  try
  {
    os = AppendFile("/tmp/notexist");
  }
  catch (const FileSystemError& e)
  {
    std::cout << "append: " << e.what() << " " << e.Errno() << std::endl;
  }
  
  InStreamPtr is(OpenFile("/tmp/newfile"));
  
  std::string line;
  while (std::getline(*is, line)) std::cout << line << std::endl;
}

#endif
