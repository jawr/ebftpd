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

#include <unistd.h>
#include <cerrno>
#include <sys/statvfs.h>
#include "util/path/status.hpp"
#include "util/error.hpp"

namespace util { namespace path
{

Status::Status() :
  linkDirectory(false),
  linkRegularFile(false),
  statOkay(false)
{
}

Status::Status(const std::string& path) :
  path(path),
  linkDirectory(false),
  linkRegularFile(false),
  statOkay(false)
{
  Reset();
}

Status& Status::Reset()
{
  if (path.empty()) throw std::logic_error("no path set");
  if (!statOkay)
  {    
    if (lstat(path.c_str(), &native) < 0) throw util::SystemError(errno);
    
    if (IsSymLink())
    {
      struct stat st;
      if (!stat(path.c_str(), &st) < 0) throw util::SystemError(errno);
      if (S_ISDIR(st.st_mode)) linkDirectory = true;
      else if (S_ISREG(st.st_mode)) linkRegularFile = true;
    }    
    statOkay = true;
  }
  return *this;
}

Status& Status::Reset(const std::string& path)
{
  statOkay = false;
  this->path = path;
  Reset();
  return *this;
}

bool Status::IsRegularFile() const
{
  return S_ISREG(native.st_mode) || linkRegularFile;
}

bool Status::IsDirectory() const
{
  return S_ISDIR(native.st_mode) || linkDirectory;
}

bool Status::IsSymLink() const
{
  return S_ISLNK(native.st_mode);
}

bool Status::IsExecutable() const
{
  return native.st_mode & S_IXOTH ||
         (geteuid() == native.st_uid && (native.st_mode & S_IXUSR)) ||
         (getegid() == native.st_gid && (native.st_mode & S_IXGRP));
}

bool Status::IsWriteable() const
{
  return native.st_mode & S_IWOTH ||
         (geteuid() == native.st_uid && (native.st_mode & S_IWUSR)) ||
         (getegid() == native.st_gid && (native.st_mode & S_IWGRP));
}

bool Status::IsReadable() const
{
  return native.st_mode & S_IROTH ||
         (geteuid() == native.st_uid && (native.st_mode & S_IRUSR)) ||
         (getegid() == native.st_gid && (native.st_mode & S_IRGRP));
}

off_t Status::Size() const
{
  return native.st_size;
}

time_t Status::AccessTime() const
{
  return native.st_atime;
}

time_t Status::ModTime() const
{
  return native.st_mtime;
}

time_t Status::ChangeTime() const
{
  return native.st_ctime;
}

const struct stat& Status::Native() const
{
  return native;
}

bool Exists(const std::string& path)
{
  try { (void) Status(path); } catch (const util::SystemError&) { return false; }
  return true;
}

bool IsDirectory(const std::string& path)
{
  try { return Status(path).IsDirectory(); } catch (const util::SystemError&) { }
  return false;
}

bool IsRegularFile(const std::string& path)
{
  try { return Status(path).IsRegularFile(); } catch (const util::SystemError&) { }
  return false;
}

bool IsSymLink(const std::string& path)
{
  try { return Status(path).IsSymLink(); } catch (const util::SystemError&) { }
  return false;
}

off_t Size(const std::string& path)
{
  try { return Status(path).Size(); } catch (const util::SystemError&) { }
  return -1;
}

time_t ModTime(const std::string& path)
{
  try { return Status(path).ModTime(); } catch (const util::SystemError&) { }
  return -1;  
}

util::Error FreeDiskSpace(const std::string& real, unsigned long long& freeBytes)
{
  struct statvfs sfs;
  if (statvfs(real.c_str(), &sfs) <0)
    return util::Error::Failure(errno);

  freeBytes = static_cast<unsigned long long>(sfs.f_bsize) * 
              static_cast<unsigned long long>(sfs.f_bavail);
  return util::Error::Success();
}

} /* path namespace */
} /* util namespace */
