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

#ifndef __UTIL_INTERRUPT_PIPE_HPP
#define __UTIL_INTERRUPT_PIPE_HPP

#include "util/pipe.hpp"

namespace util
{

class InterruptPipe : public Pipe
{
public:
  void Acknowledge()
  {
    char ch;
    (void) read(ReadFd(), &ch, 1);
  }

  void Interrupt()
  {
    (void) write(WriteFd(), "", 1);
  }
};

} /* util namespace */

#endif
