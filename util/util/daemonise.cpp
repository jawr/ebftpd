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

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <cassert>
#include <unistd.h>
#include <csignal>
#include <fstream>
#include <sys/file.h>
#include "util/daemonise.hpp"

namespace util { namespace daemonise
{

// this pid file code should probably be changed over to 
// using a file lock some time in the future

util::Error NotRunning(const std::string& pidFile)
{
  typedef boost::iostreams::stream<
          boost::iostreams::file_descriptor_source> InStream;

  int fd = open(pidFile.c_str(), O_RDONLY);
  if (fd < 0)
  {
    if (errno == ENOENT) return util::Error::Success();
    else return util::Error::Failure(errno);
  }
  
  InStream is(fd, boost::iostreams::close_handle);
  pid_t pid;
  is >> pid;
  if (is.fail() || is.bad()) return util::Error::Failure("Error while reading pid file.");
  is.close();
  
  if (kill(pid, 0) < 0)
  {
    if (errno == ESRCH) return util::Error::Success();
    return util::Error::Failure(errno);
  }
  
  return util::Error::Failure("Process already running");
}

util::Error CreatePIDFile(const std::string& pidFile)
{
  typedef boost::iostreams::stream<
          boost::iostreams::file_descriptor_sink> OutStream;

  int fd = open(pidFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0) return util::Error::Failure(errno);
  
  OutStream os(fd, boost::iostreams::close_handle);
  os << getpid();
  if (os.fail() || os.bad()) return util::Error::Failure("Error while writing pid file.");
  os.close();
  
  return util::Error::Success();
}

util::Error Daemonise()
{
  pid_t pid = fork();
  if (pid < 0) return util::Error::Failure(errno);
  if (pid > 0) _exit(0);
  
  setsid();
  
  std::cout.setstate(std::ios::failbit);
  std::cerr.setstate(std::ios::failbit);
  std::cin.setstate(std::ios::failbit);
  std::clog.setstate(std::ios::failbit);
  
  return util::Error::Success();
}

} /* daemonise namespace */
} /* util namespace */
