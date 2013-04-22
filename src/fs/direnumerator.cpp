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

#include <cassert>
#include <memory>
#include <cstring>
#include <dirent.h>
#include "fs/direnumerator.hpp"
#include "acl/user.hpp"
#include "acl/path.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "logs/logs.hpp"

namespace fs
{

DirEnumerator::DirEnumerator() :
  user(nullptr),
  totalBytes(0),
  loadOwners(true)
{
}

DirEnumerator::DirEnumerator(const fs::Path& path, bool loadOwners) :
  user(nullptr),
  path(path),
  totalBytes(0),
  loadOwners(loadOwners)
{
  Readdir();
}

DirEnumerator::DirEnumerator(const acl::User& user, const fs::VirtualPath& path, bool loadOwners) :
  user(&user),
  path(MakeReal(path)),
  totalBytes(0),
  loadOwners(loadOwners)
{
  Readdir();
}

void DirEnumerator::Readdir(const fs::Path& path, bool loadOwners)
{
  this->path = RealPath(path);
  this->loadOwners = loadOwners;
  Readdir();
}

void DirEnumerator::Readdir(const acl::User& user, const fs::VirtualPath& path, bool loadOwners)
{
  this->user  = &user;
  this->path = MakeReal(path);
  this->loadOwners = loadOwners;
  Readdir();
}

void DirEnumerator::Readdir()
{
  namespace PP = acl::path;

  if (user && !PP::DirAllowed<PP::View>(*user, MakeVirtual(path))) 
  {
    return;
  }

  DIR* dp = opendir(path.CString());
  if (!dp) throw util::SystemError(errno);
  std::shared_ptr<DIR> dpGuard(dp, closedir);
  
  struct dirent de;
  struct dirent* dep;
  while (true)
  {
    readdir_r(dp, &de, &dep);
    if (!dep) break;

     if (!strcmp(de.d_name, ".") || !strcmp(de.d_name, "..")) continue;

    fs::RealPath entryPath(path / de.d_name);

    try
    {
      util::path::Status status(entryPath.ToString());
      totalBytes += status.Size();
      
      if (user)
      {
        fs::VirtualPath virtPath(MakeVirtual(entryPath));
        util::Error hideOwner;
        if (status.IsDirectory())
        {
          if (!PP::DirAllowed<PP::View>(*user, virtPath)) continue;
          hideOwner = PP::DirAllowed<PP::Hideowner>(*user, virtPath);
        }
        else
        {
          if (!PP::FileAllowed<PP::View>(*user, virtPath)) continue;
          hideOwner = PP::FileAllowed<PP::Hideowner>(*user, virtPath);
        }
        
        Owner owner(0, 0);
        if (!hideOwner && loadOwners) owner = GetOwner(entryPath);
        entries.emplace_back(fs::Path(de.d_name), status, owner);
      }
      else
      {
        Owner owner(0, 0);
        if (loadOwners) owner = GetOwner(entryPath);
        entries.emplace_back(fs::Path(de.d_name), status, owner);
      }
    }
    catch (const util::SystemError&)
    {
      continue;
    }
  }
}

} /* fs namespace */
