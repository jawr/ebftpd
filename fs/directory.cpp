#include <sys/stat.h>
#include <cerrno>
#include <cassert>
#include "directory.hpp"
#include "status.hpp"

namespace fs
{

Error CreateDirectory(const std::string& path)
{
  if (mkdir(path.c_str(), 0777) < 0) return Error::Failure(errno);
  else return Error::Success();
}

Error CreateDirectory(const User& user, const std::string& path)
{
  // check ACLS here
  return CreateDirectory(path);
}

Error CreateDirectory(const Client& client, const std::string& path)
{
  // take current workdir from client and resolve it against path
  // to get a an absolute path, then pass client->user and absolute
  // path below;
  // return CreateDirectory(client->User(), absolutePath);
  assert(false);
}

Error RemoveDirectory(const std::string& path)
{
  if (rmdir(path.c_str()) < 0) return Error::Failure(errno);
  else return Error::Success();
}

Error RemoveDirectory(const User& user, const std::string& path)
{
  // check ACLs here
  return RemoveDirectory(path);
}

Error RemoveDirectory(const Client& client, const std::string& path)
{
  // take current workdir from client and resolve it against path
  // to get a an absolute path, then pass client->user and absolute
  // path below;
  // return RemoveDirectory(client->User(), absolutePath);
  assert(false);
}

// we don't literally change the the working dir of the process
// as the ftpd is multithreaded and this would change work dir
// of all threads, we instead just check that chdir is possible
Error ChangeDirectory(const std::string& path)
{
  try
  {
    Status stat(path);
    if (!stat.IsDirectory()) return Error::Failure(ENOTDIR);
    if (!stat.IsExecutable()) return Error::Failure(EACCES);
  }
  catch (const FileSystemError& e)
  {
    if (e.ValidErrno()) return Error::Failure(e.Errno());
    else return Error::Failure();
  }
  
  return Error::Success();
}

Error ChangeDirectory(const User& user, const std::string& path)
{
  // check ACLs here
  return ChangeDirectory(path);
}

Error ChangeDirectory(const Client& client, const std::string& path)
{
  // take current workdir from client and resolve it against path
  // to get a an absolute path, then pass client->user and absolute
  // path below;
  // return ChangeDirectory(client->User(), absolutePath);
  assert(false);
}

} /* fs namespace */

#ifdef FS_DIRECTORY_TEST

#include <iostream>

int main()
{
  using namespace fs;
  
  Error e = CreateDirectory("/tmp");
  if (!e) std::cout << "mkdir failed: " << e.Message() << std::endl;
  else std::cout << "mkdir okay" << std::endl;
  
  e = RemoveDirectory("/tmp/something");
  if (!e) std::cout << "rmdir failed: " << e.Message() << std::endl;
  else std::cout << "rmdir okay" << std::endl;
  
  e = ChangeDirectory("/tmp");
  if (!e) std::cout << "chdir failed: " << e.Message() << std::endl;
  else std::cout << "chdir okay" << std::endl;

  e = ChangeDirectory("/tmp/notexist");
  if (!e) std::cout << "chdir failed: " << e.Message() << std::endl;
  else std::cout << "chdir okay" << std::endl;
  
}

#endif
