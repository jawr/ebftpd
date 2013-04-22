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

#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include "fs/filelock.hpp"
#include "util/error.hpp"

namespace fs
{

FileLock::FileLock(const std::string& path)
{
  fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) throw util::SystemError(errno);
  if (flock(fd, LOCK_EX) < 0)
  {
    int errno_ = errno;
    close(fd);
    fd = -1;
    throw util::SystemError(errno_);
  }
}

FileLock::~FileLock()
{
  if (fd >= 0)
  {
    flock(fd, LOCK_UN);
    close(fd);
  }
}

std::shared_ptr<FileLock> FileLock::Create(const std::string& path)
{
  return std::shared_ptr<FileLock>(new FileLock(path));
}

} /* fs namespace */
