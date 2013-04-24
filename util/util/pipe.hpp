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
