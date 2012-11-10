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
#include "fs/dircontainer.hpp"

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

util::Error ChangeDirectory(ftp::Client& client, const Path& path)
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
    return util::Error::Failure(errno);
  }
  
  client.SetWorkDir(absolute);
  return util::Error::Success();
}

util::Error ChangeAlias(ftp::Client& client, fs::Path& path)
{
  std::string name = boost::to_lower_copy(path.ToString());
  for (auto& alias : cfg::Get().Alias())
  {
    if (alias.Name() == name)
    {
      path = alias.Path();
      return ChangeDirectory(client, alias.Path());
    }
  }
  
  return util::Error::Failure(ENOENT);
}

util::Error ChangeMatch(ftp::Client& client, Path& path)
{
  Path orig(path);
  Path absolute = (client.WorkDir() / path).Expand();

  util::Error e(PP::DirAllowed<PP::Makedir>(client.User(), absolute));
  if (!e) return e;
    
  Path real = cfg::Get().Sitepath() + absolute;

  try
  {
    fs::Status status;
    for (auto& entry : fs::DirContainer(real.Dirname()))
    {
      try
      {
        status.Reset(real.Dirname() + entry);
      }
      catch (const util::SystemError& e)
      { continue; }
      
      if (boost::istarts_with(entry, path.Basename().ToString()) &&
          status.IsDirectory())
      {
        path = absolute.Dirname() / entry;
        if (!PP::DirAllowed<PP::View>(client.User(), path))
          return util::Error::Failure(ENOENT);

        if (!status.IsExecutable()) return util::Error::Failure(EACCES);
        else return util::Error::Success();
      }      
    }
  }
  catch (const util::SystemError& e)
  {
    return util::Error::Failure(e.Errno());
  }
  
  if (orig == orig.Basename())
  {
    for (auto& cdpath : cfg::Get().Cdpath())
    {
      path = cdpath / orig;
      logs::debug << path << logs::endl;
      e = ChangeMatch(client, path);
      if (e || e.Errno() != ENOENT) return e;
    }
  }

  return util::Error::Failure(ENOENT);
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
