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
  
  return util::Error::Success();
}

} /* daemonise namespace */
} /* util namespace */
