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
#include "logger/logger.hpp"
#include "acl/check.hpp"
#include "cfg/get.hpp"

#include <iostream>

namespace PP = acl::PathPermission;

namespace fs
{
namespace 
{

util::Error RemoveDirectory(const Path& path)
{
  Path real = cfg::Get()->Sitepath() + path;
  
  DirEnumerator dirEnum;
  
  try
  {
    dirEnum.Readdir(real);
  }
  catch (const util::SystemError& e)
  {
    return util::Error::Failure(e.Errno());
  }
  
  for (DirEnumerator::const_iterator it =
       dirEnum.begin(); it != dirEnum.end(); ++it)
  {
    if (it->Path().ToString()[0] != '.' ||
        it->Status().IsDirectory() ||
        !it->Status().IsWriteable())
      return util::Error::Failure(ENOTEMPTY);
  }
  
  for (DirEnumerator::const_iterator it =
       dirEnum.begin(); it != dirEnum.end(); ++it)
  {
    fs::Path fullPath = real / it->Path();
    if (unlink(fullPath.CString()) < 0)
      return util::Error::Failure(errno);
    OwnerCache::Delete(fullPath);
  }
    
  logger::ftpd << "real: " << real << logger::endl;
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

  std::cout << "abs: " << absolute << std::endl;
  std::cout << "dirname: " << absolute.Dirname() << std::endl;
  

  util::Error e(PP::DirAllowed<PP::View>(client.User(), absolute));
  if (!e) return e;

  Path real = cfg::Get()->Sitepath() + path;

  std::cout << "real: " << real << std::endl;
  
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
    
    for (DirEnumerator::const_iterator it =
         dirEnum.begin(); it != dirEnum.end(); ++it)
    {
      if (boost::istarts_with(it->Path().ToString(), real.Basename().ToString()) &&
          it->Status().IsDirectory())
      {
        if (!PP::DirAllowed<PP::View>(client.User(), path))
          return util::Error::Failure(ENOENT);
        std::cout << absolute << " " << absolute.Dirname() << " " << it->Path() << std::endl;
        path = absolute = absolute.Dirname() / it->Path();
        if (!it->Status().IsExecutable()) return util::Error::Failure(EACCES);
      }
    }
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
    
  Path real = cfg::Get()->Sitepath() + absolute;
  if (mkdir(real.CString(), 0777) < 0) return util::Error::Failure(errno);
  
  OwnerCache::Chown(real, Owner(client.User().UID(), client.User().PrimaryGID()));
  
  return util::Error::Success();
}

util::Error RemoveDirectory(ftp::Client& client, const Path& path)
{
  Path absolute = (client.WorkDir() / path).Expand();
  
  util::Error e(PP::DirAllowed<PP::Delete>(client.User(), absolute));
  if (!e) return e;
  
  logger::ftpd << "absolute: " << absolute << logger::endl;
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
