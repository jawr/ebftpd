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

#include <cstdio>
#include <cerrno>
#include <sys/stat.h>
#include <fcntl.h>
#include "fs/file.hpp"
#include "acl/user.hpp"
#include "fs/path.hpp"
#include "fs/owner.hpp"
#include "util/misc.hpp"
#include "acl/path.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "util/randomstring.hpp"
#include "util/path/status.hpp"
#include "util/error.hpp"
#include "logs/logs.hpp"

namespace PP = acl::path;

namespace fs
{

util::Error DeleteFile(const RealPath& path)
{
  if (unlink(path.CString()) < 0) return util::Error::Failure(errno);
  return util::Error::Success();
}

util::Error DeleteFile(const acl::User& user, const VirtualPath& path, 
      off_t* size, time_t* modTime)
{
  util::Error e = PP::FileAllowed<PP::Delete>(user, path);
  if (!e) return e;
  
  if (size)
  {
    try
    {
      util::path::Status status(MakeReal(path).ToString());
      *size = status.Size();
      *modTime = status.Native().st_mtime;
    }
    catch (const util::SystemError& e)
    {
      return util::Error::Failure(e.Errno());
    }
  }

  return DeleteFile(MakeReal(path));
}

util::Error Rename(const RealPath& oldPath, const RealPath& newPath)
{
  if (rename(oldPath.CString(), newPath.CString()) < 0) 
    return util::Error::Failure(errno);
  return util::Error::Success();
}

FileSinkPtr CreateFile(const acl::User& user, const VirtualPath& path)
{
  util::Error e(PP::FileAllowed<PP::Upload>(user, path));
  if (!e) throw util::SystemError(e.Errno());
  
  unsigned long long freeBytes;
  e = util::path::FreeDiskSpace(MakeReal(path).Dirname().ToString(), freeBytes);
  if (!e) throw util::SystemError(e.Errno());
  
  if (static_cast<unsigned long long>(cfg::Get().FreeSpace()) > freeBytes / 1024)
    throw util::SystemError(ENOSPC);

  mode_t mode = cfg::Get().DlIncomplete() ? 0755 : 0644;
    
  int fd = open(MakeReal(path).CString(), O_CREAT | O_WRONLY | O_EXCL, mode);
  if (fd < 0)
  {
    if (errno != EEXIST) throw util::SystemError(errno);

    e = PP::FileAllowed<PP::Overwrite>(user, path);
    if (!e) throw util::SystemError(EEXIST);
    
    fd = open(MakeReal(path).CString(), O_WRONLY | O_TRUNC);
    if (fd < 0) throw util::SystemError(errno);
  }

  SetOwner(MakeReal(path), Owner(user.ID(), user.PrimaryGID()));

  return std::make_shared<FileSink>(fd, boost::iostreams::close_handle);
}

FileSinkPtr AppendFile(const acl::User& user, const VirtualPath& path, off_t offset)
{
  util::Error e = PP::FileAllowed<PP::Resume>(user, path);
  if (!e) throw util::SystemError(e.Errno());

  auto real = MakeReal(path);
  try
  {
    if (!util::path::Status(real.ToString()).IsRegularFile())
      throw util::RuntimeError("Not a regular file");
  }
  catch (const util::SystemError& e)
  {
    throw util::SystemError(e.Errno());
  }

  unsigned long long freeBytes;
  e = util::path::FreeDiskSpace(real.Dirname().ToString(), freeBytes);
  if (!e) throw util::SystemError(e.Errno());
  
  if (static_cast<unsigned long long>(cfg::Get().FreeSpace()) > freeBytes / 1024)
    throw util::SystemError(ENOSPC);

  int fd = open(real.CString(), O_WRONLY | O_APPEND);
  if (fd < 0) throw util::SystemError(errno);
 
  auto fout = std::make_shared<FileSink>(fd, boost::iostreams::close_handle);

  try
  {
    std::streampos size = fout->seek(0, std::ios_base::end);
    if (offset < size && ftruncate(fout->handle(), offset) < 0)
      throw util::SystemError(errno);
    fout->seek(0, std::ios_base::end);  
  }
  catch (const std::ios_base::failure& e)
  {
    throw util::SystemError(errno);
  }
  
  return fout;
}

FileSourcePtr OpenFile(const acl::User& user, const VirtualPath& path)
{
  util::Error e = PP::FileAllowed<PP::Download>(user, path);
  if (!e) throw util::SystemError(e.Errno());
  
  auto real = MakeReal(path);
  try
  {
    if (!util::path::Status(real.ToString()).IsRegularFile())
      throw util::RuntimeError("Not a regular file");
  }
  catch (const util::SystemError& e)
  {
    throw util::SystemError(e.Errno());
  }

  int fd = open(real.CString(), O_RDONLY);
  if (fd < 0) throw util::SystemError(errno);
  return std::make_shared<FileSource>(fd, boost::iostreams::close_handle);
}

util::Error UniqueFile(const acl::User& user, const VirtualPath& path, 
                       size_t filenameLength, VirtualPath& uniquePath)
{ 
  util::Error e = PP::FileAllowed<PP::Upload>(user, path / "dummyfile");
  if (!e) throw util::SystemError(e.Errno());

  for (int i = 0; i < 1000; ++i)
  {
    std::string filename = util::RandomString(filenameLength, 
            util::RandomString::alphaNumeric);
    
    uniquePath = path / filename;
    try
    {
      util::path::Status status(MakeReal(uniquePath).ToString());
    }
    catch (const util::SystemError& e)
    {
      if (e.Errno() == ENOENT) return util::Error::Success();
      else return util::Error::Failure(e.Errno());
    }
  }
  
  return util::Error::Failure();
}

bool IsIncomplete(const RealPath& path)
{
  static const time_t maxInactivity = 30;
  
  try
  {
    util::path::Status status(path.ToString());
    if (status.IsExecutable() && 
        time(nullptr) - status.Native().st_mtime < maxInactivity)
      return true;
  }
  catch (const util::SystemError&)
  {
  }
  
  return false;
}

} /* fs namespace */
