#ifndef __UTIL_PIPE_HPP
#define __UTIL_PIPE_HPP

#include <unistd.h>
#include <cerrno>
#include "util/error.hpp"

namespace util
{

class Pipe
{
  int fds[2];

public:
  Pipe()
  {
    if (pipe(fds) < 0) throw SystemError(errno);
  }
  
  ~Pipe()
  {
    close(fds[0]);
    close(fds[1]);
  }

  int ReadFd() const { return fds[0]; }
  int WriteFd() const { return fds[1]; }  

  void Interrupt() const 
  {
    (void) write(WriteFd(), "1", 1);
  }
};

} /* util namespace */

#endif
