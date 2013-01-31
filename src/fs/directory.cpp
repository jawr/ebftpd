#include <sys/stat.h>
#include <cerrno>
#include <cassert>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/thread/tss.hpp>
#include "fs/directory.hpp"
#include "fs/status.hpp"
#include "acl/user.hpp"
#include "ftp/client.hpp"
#include "fs/owner.hpp"
#include "fs/direnumerator.hpp"
#include "acl/path.hpp"
#include "cfg/get.hpp"
#include "fs/dircontainer.hpp"
#include "fs/path.hpp"

namespace PP = acl::path;

namespace fs
{
namespace 
{
boost::thread_specific_ptr<VirtualPath> workDir;
}

const VirtualPath& WorkDirectory()
{
  VirtualPath* wd = workDir.get();
  assert(wd && "Work directory must be set before constructing AbsoltuePath and RealPath objectss");
  return *wd;
}

void SetWorkDirectory(const VirtualPath& path)
{
  workDir.reset(new VirtualPath(path));
}

util::Error ChangeDirectory(ftp::Client& client, const VirtualPath& path)
{
  util::Error e(PP::DirAllowed<PP::View>(client.User(), path));
  if (!e) return e;

  try
  {
    Status stat(MakeReal(path));
    if (!stat.IsDirectory()) return util::Error::Failure(ENOTDIR);
    if (!stat.IsExecutable()) return util::Error::Failure(EACCES);
  }
  catch (const util::SystemError& e)
  {
    return util::Error::Failure(e.Errno());
  }
  
  SetWorkDirectory(path);
  return util::Error::Success();
}

util::Error ChangeAlias(ftp::Client& client, const Path& path, VirtualPath& match)
{
  if (path.Basename() != path) return util::Error::Failure(ENOENT);
  
  std::string name = boost::to_lower_copy(path.ToString());
  for (auto& alias : cfg::Get().Alias())
  {
    if (alias.Name() == name)
    {
      match = VirtualPath(alias.Path());
      return ChangeDirectory(client, match);
    }
  }
  
  return util::Error::Failure(ENOENT);
}

util::Error ChangeMatch(ftp::Client& client, const VirtualPath& path, VirtualPath& match)
{
  std::string lcBasename(boost::to_lower_copy(path.Basename().ToString()));

  util::Error e(PP::DirAllowed<PP::View>(client.User(), path));
  if (!e) return e;

  try
  {
    for (auto& entry : DirContainer(client, path.Dirname()))
    {
      if (!boost::starts_with(boost::to_lower_copy(entry.ToString()), lcBasename)) continue;
      match = path.Dirname() / entry;
      e = ChangeDirectory(client, match);
      if (e || (e.Errno() != ENOENT && e.Errno() != ENOTDIR))
      {
        return e;
      }
    }
  }
  catch (const util::SystemError& e)
  {
    return util::Error::Failure(e.Errno());
  }

  return util::Error::Failure(ENOENT);
}

util::Error ChangeCdpath(ftp::Client& client, const fs::Path& path, VirtualPath& match)
{
  if (path.Basename() != path)  return util::Error::Failure(ENOENT);
    
  for (auto& cdpath : cfg::Get().Cdpath())
  {
    util::Error e(ChangeMatch(client, MakeVirtual(cdpath / path), match));
    if (e || e.Errno() != ENOENT) return e;
  }
  
  return util::Error::Failure(ENOENT);
}

util::Error CreateDirectory(const RealPath& path)
{
  if (mkdir(MakeReal(path).CString(), 0777) < 0) return util::Error::Failure(errno);
  return util::Error::Success();
}

util::Error CreateDirectory(ftp::Client& client, const VirtualPath& path)
{
  util::Error e(PP::DirAllowed<PP::Makedir>(client.User(), path));
  if (!e) return e;

  e = CreateDirectory(MakeReal(path));  
  if (e) OwnerCache::Chown(MakeReal(path), Owner(client.User().UID(), 
        client.User().PrimaryGID()));
  return e;
}

util::Error RemoveDirectory(const RealPath& path)
{
  if (rmdir(MakeReal(path).CString()) < 0) return util::Error::Failure(errno);
  OwnerCache::Delete(MakeReal(path));
  return util::Error::Success();
}

util::Error RemoveDirectory(ftp::Client& client, const VirtualPath& path)
{
  util::Error e(PP::DirAllowed<PP::Delete>(client.User(), path));
  if (!e) return e;
  
  try
  {
    DirContainer dirCont(MakeReal(path));
    for (auto& name : dirCont)
    {
      if (name.ToString()[0] !=  '.') return
        util::Error::Failure(ENOTEMPTY);
        
      Status status(MakeReal(path) / name);
      if (status.IsDirectory() ||
          !status.IsWriteable())
        return util::Error::Failure(ENOTEMPTY);
    }
    
    
    dirCont.Rewind();
    
    for (auto& name : dirCont)
    {
      RealPath entryPath(MakeReal(path) / name);
      if (unlink(entryPath.CString()) < 0)
        return util::Error::Failure(errno);
      OwnerCache::Delete(entryPath);      
    }
  }
  catch (const util::SystemError& e)
  {
    return util::Error::Failure(e.Errno());
  }
    
  return RemoveDirectory(MakeReal(path));
}

util::Error RenameDirectory(const RealPath& oldPath, const RealPath& newPath)
{
  OwnerCache::Flush(oldPath);

  if (rename(oldPath.CString(), newPath.CString()) < 0) 
    return util::Error::Failure(errno);
    
  return util::Error::Success();
}

util::Error RenameDirectory(ftp::Client& client, const VirtualPath& oldPath,
                 const VirtualPath& newPath)                 
{
  util::Error e = PP::DirAllowed<PP::Rename>(client.User(), oldPath);
  if (!e) return e;

  e = PP::DirAllowed<PP::Makedir>(client.User(), newPath);
  if (!e) return e;
  
  return RenameDirectory(MakeReal(oldPath), MakeReal(newPath));
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
