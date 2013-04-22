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

#ifndef __UTIL_REVERSE_LOG_READER_HPP
#define __UTIL_REVERSE_LOG_READER_HPP

#include <string>
#include <cassert>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "util/error.hpp"

namespace util
{

class ReverseLogReader
{
  int fd;
  char* data;
  off_t pos;
  
  void Open(const std::string& path)
  {
    struct stat st;
    if (stat(path.c_str(), &st) < 0) throw util::SystemError(errno);
    if (st.st_size == 0) return;

    fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) throw util::SystemError(errno);

    data = reinterpret_cast<char*>(mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0));
    if (!data) throw util::SystemError(errno);
    pos = st.st_size;
    
    while (pos > -1 && (data[pos] == '\r'  ||  data[pos] == '\n' || data[pos] == '\0'))
    {
      --pos;
    }
  }
  
public:
  ReverseLogReader(const std::string& path) :
    fd(-1), data(nullptr), pos(-1)
  {
    Open(path);
  }
  
  ~ReverseLogReader()
  {
    if (fd >= 0) close(fd);
  }
  
  bool Getline(std::string& line)
  {
    if (pos == -1) return false;

    off_t lastPos = pos;
    for (--pos; pos > -1; --pos)
      if (data[pos] == '\n') break;
      
    line = std::string(data + pos + 1, lastPos - pos);
    if (pos != -1 && data[--pos] == '\r') --pos;
    return true;
  }
};

} /* util namespace */

#endif
