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

#ifndef __FS_STATUS_HPP
#define __FS_STATUS_HPP

#include <string>
#include <sys/stat.h>

namespace util
{
class Error;
}

namespace util { namespace path
{

class Status
{
  std::string path;
  struct stat native;
  bool linkDirectory;
  bool linkRegularFile;
  bool statOkay;
  
  Status& Reset();
  
public:
  Status();
  Status(const std::string& path);
  
  Status& Reset(const std::string& path);
  
  bool IsRegularFile() const;
  bool IsDirectory() const;
  bool IsSymLink() const;
  
  bool IsExecutable() const;
  bool IsReadable() const;
  bool IsWriteable() const;
  uid_t UID() const { return native.st_uid; }
  gid_t GID() const { return native.st_gid; }
  
  off_t Size() const;
  
  time_t AccessTime() const;
  time_t ModTime() const;
  time_t ChangeTime() const;
  
  const struct stat& Native() const;
};

bool IsDirectory(const std::string& path);
bool IsRegularFile(const std::string& path);
bool IsSymLink(const std::string& path);
off_t Size(const std::string& path);
time_t ModTime(const std::string& path);

util::Error FreeDiskSpace(const std::string& real, unsigned long long& freeBytes);

} /* path namespace */
} /* fs namespace */

#endif
