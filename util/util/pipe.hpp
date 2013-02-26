#ifndef __UTIL_PIPE_HPP
#define __UTIL_PIPE_HPP

#include <unistd.h>
#include <cerrno>
#include <boost/noncopyable.hpp>
#include "util/error.hpp"

namespace util
{

class Pipe : boost::noncopyable
{
  int fds[2];

public:
  Pipe()
  {
    if (pipe(fds) < 0) throw SystemError(errno);
  }
  
  int ReadFd() const { return fds[0]; }
  int WriteFd() const { return fds[1]; }  
  
  void CloseRead()
  {
    if (fds[0] >= 0)
    {
      close(fds[0]); 
      fds[0] = -1;
    }
  }
  
  void CloseWrite()
  {
    if (fds[1] >= 0)
    {
      close(fds[1]);
      fds[1] = -1;
    }
  }
  
  void Close()
  {
    CloseRead();
    CloseWrite();
  }
  
  ~Pipe()
  {
    Close();
  }
  
  void Reset()
  {
    Close();
    if (pipe(fds) < 0) throw SystemError(errno);
  }
};

} /* util namespace */

#endif
