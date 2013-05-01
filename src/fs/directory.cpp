//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <sys/stat.h>
#include <cerrno>
#include <cassert>
#include <boost/thread/tss.hpp>
#include "fs/directory.hpp"
#include "util/path/status.hpp"
#include "acl/user.hpp"
#include "fs/owner.hpp"
#include "fs/direnumerator.hpp"
#include "acl/path.hpp"
#include "cfg/get.hpp"
#include "fs/dircontainer.hpp"
#include "util/path/dircontainer.hpp"
#include "fs/path.hpp"
#include "util/error.hpp"
#include "util/string.hpp"

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

util::Error ChangeDirectory(const acl::User& user, const VirtualPath& path)
{
  util::Error e(PP::DirAllowed<PP::View>(user, path));
  if (!e) return e;

  try
  {
    util::path::Status stat(MakeReal(path).ToString());
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

util::Error ChangeAlias(const acl::User& user, const Path& path, VirtualPath& match)
{
  if (path.Basename() != path) return util::Error::Failure(ENOENT);
  
  std::string name = util::ToLowerCopy(path.ToString());
  for (auto& alias : cfg::Get().Alias())
  {
    if (alias.Name() == name)
    {
      match = VirtualPath(alias.Path());
      return ChangeDirectory(user, match);
    }
  }
  
  return util::Error::Failure(ENOENT);
}

util::Error ChangeMatch(const acl::User& user, const VirtualPath& path, VirtualPath& match)
{
  std::string lcBasename(util::ToLowerCopy(path.Basename().ToString()));

  util::Error e(PP::DirAllowed<PP::View>(user, path));
  if (!e) return e;

  try
  {
    for (auto& entry : DirContainer(user, path.Dirname()))
    {
      if (!util::StartsWith(util::ToLowerCopy(entry), lcBasename)) continue;
      // this hackishness forces creation of a fresh virtualpath,
      // therefore forcing resolution of symlinks
      match = MakeVirtual(fs::Path((path.Dirname() / entry).ToString()));
      e = ChangeDirectory(user, match);
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

util::Error ChangeCdpath(const acl::User& user, const fs::Path& path, VirtualPath& match)
{
  if (path.Basename() != path)  return util::Error::Failure(ENOENT);
    
  for (auto& cdpath : cfg::Get().Cdpath())
  {
    util::Error e(ChangeMatch(user, MakeVirtual(cdpath / path), match));
    if (e || e.Errno() != ENOENT) return e;
  }
  
  return util::Error::Failure(ENOENT);
}

util::Error CreateDirectory(const RealPath& path)
{
  if (mkdir(MakeReal(path).CString(), 0777) < 0) return util::Error::Failure(errno);
  return util::Error::Success();
}

util::Error CreateDirectory(const acl::User& user, const VirtualPath& path)
{
  util::Error e(PP::DirAllowed<PP::Makedir>(user, path));
  if (!e) return e;

  e = CreateDirectory(MakeReal(path));  
  if (e) SetOwner(MakeReal(path), Owner(user.ID(), user.PrimaryGID()));
  return e;
}

util::Error RemoveDirectory(const RealPath& path)
{
  if (rmdir(MakeReal(path).CString()) < 0) return util::Error::Failure(errno);
  return util::Error::Success();
}

util::Error RemoveDirectory(const acl::User& user, const VirtualPath& path)
{
  util::Error e(PP::DirAllowed<PP::Delete>(user, path));
  if (!e) return e;
  
  try
  {
    util::path::DirContainer dirCont(MakeReal(path).ToString());
    for (auto& name : dirCont)
    {
      if (name[0] !=  '.') return util::Error::Failure(ENOTEMPTY);
        
      util::path::Status status((MakeReal(path) / name).ToString());
      if (status.IsDirectory() || !status.IsWriteable())
        return util::Error::Failure(ENOTEMPTY);
    }
    
    
    for (auto& name : dirCont)
    {
      RealPath entryPath(MakeReal(path) / name);
      if (unlink(entryPath.CString()) < 0)
        return util::Error::Failure(errno);
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
  if (rename(oldPath.CString(), newPath.CString()) < 0) 
    return util::Error::Failure(errno);
    
  return util::Error::Success();
}

util::Error DirectorySize(const RealPath& path, int depth, long long& kBytes, bool ignoreHidden)
{
  kBytes = 0;
  if (depth < 0) return util::Error::Failure(EINVAL);
  if (depth == 0) return util::Error::Success();
  
  try
  {
    for (auto& entry : util::path::DirContainer(path.ToString()))
    {
      if (ignoreHidden && entry[0] == '.') continue;
      try
      {
        auto entryPath = path / entry;
        util::path::Status status(entryPath.ToString());
        if (status.IsDirectory())
        {
          if (!status.IsSymLink())
          {
            long long subKBytes;
            if (DirectorySize(entryPath, depth - 1, subKBytes, ignoreHidden))
              kBytes += subKBytes;
          }
        }
        else
        if (status.IsRegularFile())
        {
          kBytes += status.Size() / 1024;
        }
      }
      catch (const util::SystemError& e)
      { }
    }
  }
  catch (const util::SystemError& e)
  {
    return util::Error::Failure(e.Errno());
  }
  
  return util::Error::Success();
}

} /* fs namespace */
