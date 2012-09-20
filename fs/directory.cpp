#include <sys/stat.h>
#include <cerrno>
#include <cassert>
#include "directory.hpp"
#include "status.hpp"
#include "acl/user.hpp"
#include "ftp/client.hpp"

#include <iostream>

namespace fs
{
namespace
{

const Path dummySiteRoot = "/home/bioboy/ftpd/site";

util::Error CreateDirectory(const Path& path)
{
  Path real = dummySiteRoot + path;
  if (mkdir(real.CString(), 0777) < 0) return util::Error::Failure(errno);
  else return util::Error::Success();
}

util::Error RemoveDirectory(const Path& path)
{
  Path real = dummySiteRoot + path;
  if (rmdir(path.CString()) < 0) return util::Error::Failure(errno);
  else return util::Error::Success();
}

// we don't literally change the the working dir of the process
// as the ftpd is multithreaded and this would change work dir
// of all threads, we instead just check that chdir is possible
util::Error ChangeDirectory(const Path& path)
{
  Path real = dummySiteRoot + path;
  
  try
  {
    Status stat(real);
    if (!stat.IsDirectory()) return util::Error::Failure(ENOTDIR);
    if (!stat.IsExecutable()) return util::Error::Failure(EACCES);
  }
  catch (const util::SystemError& e)
  {
    if (e.ValidErrno()) return util::Error::Failure(e.Errno());
    else return util::Error::Failure();
  }
  
  return util::Error::Success();
}

}

util::Error CreateDirectory(const ftp::Client& client, const Path& path)
{
  Path absolute = client.WorkDir() / path;
  
  // check ACLs here
  
  return CreateDirectory(absolute);
}

util::Error RemoveDirectory(const ftp::Client& client, const Path& path)
{
  Path absolute = client.WorkDir() / path;
  
  // check ACLs here
  
  return RemoveDirectory(absolute);
}

util::Error ChangeDirectory(const ftp::Client& client, const Path& path)
{
  Path absolute = client.WorkDir() / path;
  
  // check ACLs here
  
  return ChangeDirectory(absolute);
}

} /* fs namespace */

#ifdef FS_DIRECTORY_TEST

#include <iostream>

int main()
{
  using namespace fs;
  
  util::Error e = CreateDirectory("/tmp");
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

  ftp::Client client;
  std::cout << "mkdir: " << CreateDirectory(client, "/w000t").Message() << std::endl;
  std::cout << "chdir: " << ChangeDirectory(client, "/w000t").Message() << std::endl;
  std::cout << "chdir: " << ChangeDirectory(client, "/something").Message() << std::endl;
  std::cout << "rmdir: " << RemoveDirectory(client, "/w000t").Message() << std::endl;
  std::cout << "mkdir: " << CreateDirectory(client, "wow").Message() << std::endl;
  std::cout << "rmdir: " << RemoveDirectory(client, "wow").Message() << std::endl;
}

#endif
