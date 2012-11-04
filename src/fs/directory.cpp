#include <sys/stat.h>
#include <cerrno>
#include <cassert>
#include <boost/algorithm/string/predicate.hpp>
#include "fs/directory.hpp"
#include "fs/status.hpp"
#include "acl/user.hpp"
#include "ftp/client.hpp"
#include "fs/owner.hpp"
#include "fs/direnumerator.hpp"
#include "logs/logs.hpp"
#include "acl/path.hpp"
#include "cfg/get.hpp"

namespace PP = acl::path;

namespace fs
{
namespace 
{

util::Error RemoveDirectory(const Path& path)
{
  Path real = cfg::Get().Sitepath() + path;
  
  DirEnumerator dirEnum;
  
  try
  {
    dirEnum.Readdir(real);
  }
  catch (const util::SystemError& e)
  {
    return util::Error::Failure(e.Errno());
  }
  
  for (const DirEntry& de : dirEnum)
  {
    if (de.Path().ToString()[0] != '.' ||
        de.Status().IsDirectory() ||
        !de.Status().IsWriteable())
      return util::Error::Failure(ENOTEMPTY);
  }
  
  for (const DirEntry& de : dirEnum)
  {
    fs::Path fullPath = real / de.Path();
    if (unlink(fullPath.CString()) < 0)
      return util::Error::Failure(errno);
    OwnerCache::Delete(fullPath);
  }
    
  if (rmdir(real.CString()) < 0) return util::Error::Failure(errno);
  OwnerCache::Delete(real);
  return util::Error::Success();
}

}

// we don't literally change the the working dir of the process
// as the ftpd is multithreaded and this would change work dir
// of all threads, we instead just check that chdir is possible
util::Error ChangeDirectory(ftp::Client& client, Path& path)
{
  Path absolute = (client.WorkDir() / path).Expand();

  util::Error e(PP::DirAllowed<PP::View>(client.User(), absolute));
  if (!e) return e;

  Path real = cfg::Get().Sitepath() + absolute;
  try
  {
    Status stat(real);
    if (!stat.IsDirectory()) return util::Error::Failure(ENOTDIR);
    if (!stat.IsExecutable()) return util::Error::Failure(EACCES);
  }
  catch (const util::SystemError& e)
  {
    if (e.Errno() != ENOENT) return util::Error::Failure(e.Errno());

    DirEnumerator dirEnum;    
    try
    {
      dirEnum.Readdir(real.Dirname());
    }
    catch (const util::SystemError& e)
    {
      return util::Error::Failure(ENOENT);
    }
    
    bool match = false;
    for (const DirEntry& de : dirEnum)
    {
      if (boost::istarts_with(de.Path().ToString(), real.Basename().ToString()) &&
          de.Status().IsDirectory())
      {
        match = true;
        if (!PP::DirAllowed<PP::View>(client.User(), path))
          return util::Error::Failure(ENOENT);

        path = absolute = absolute.Dirname() / de.Path();
        if (!de.Status().IsExecutable()) return util::Error::Failure(EACCES);
      }
    }
    if (!match) return util::Error::Failure(ENOENT);
  }
  catch (const util::RuntimeError& e)
  {
    return util::Error::Failure();
  }
  
  client.SetWorkDir(absolute);
  return util::Error::Success();
}

util::Error CreateDirectory(ftp::Client& client, const Path& path)
{
  Path absolute = (client.WorkDir() / path).Expand();

  util::Error e(PP::DirAllowed<PP::Makedir>(client.User(), absolute));
  if (!e) return e;
    
  Path real = cfg::Get().Sitepath() + absolute;
  if (mkdir(real.CString(), 0777) < 0) return util::Error::Failure(errno);
  
  OwnerCache::Chown(real, Owner(client.User().UID(), client.User().PrimaryGID()));
  
  return util::Error::Success();
}

util::Error RemoveDirectory(ftp::Client& client, const Path& path)
{
  Path absolute = (client.WorkDir() / path).Expand();
  
  util::Error e(PP::DirAllowed<PP::Delete>(client.User(), absolute));
  if (!e) return e;
  
  return RemoveDirectory(absolute);
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
