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

#include <cstring>
#include "util/error.hpp"

namespace util
{

// this ugly hack fixes the problem with the 2 different strerror_r functions

template <typename T>
std::string ErrnoToMessage(T result, const char* buffer,
    typename std::enable_if<std::is_same<int, T>::value>::type* dummy = nullptr)
{
  if (result < 0) return "Unknown error";
  return buffer;
  (void) dummy;
}

template <typename T>
std::string ErrnoToMessage(T result, const char* /* buffer */, 
    typename std::enable_if<std::is_same<char*, T>::value>::type* dummy = nullptr)
{
  if (!result) return "Unknown error";
  return result;
  (void) dummy;
}

std::string ErrnoToMessage(int errno_)
{
  char buffer[256];
  return ErrnoToMessage(strerror_r(errno_, buffer, sizeof(buffer)), buffer);
}

SystemError::SystemError(int errno_) :
  std::runtime_error(ErrnoToMessage(errno_)),
  errno_(errno_)
{
}

} /* fs namespace */
