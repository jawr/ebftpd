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

#ifndef __UTIL_NET_THREADID_HPP
#define __UTIL_NET_THREADID_HPP

#include <pthread.h>
#include <type_traits>

namespace util { namespace net
{

class ThreadID
{
  template <typename T>
  static inline unsigned long IsPointer(const T& id, std::true_type)
  {
    return reinterpret_cast<unsigned long>(id);
  }

  template <typename T>
  static inline unsigned long IsPointer(const T& id, std::false_type)
  {
    return reinterpret_cast<unsigned long>(&id);
  }

  template <typename T>
  static inline unsigned long IsIntegral(const T& id, std::true_type)
  {
    return static_cast<unsigned long>(id);
  }

  template <typename T>
  static inline unsigned long IsIntegral(const T& id, std::false_type)
  {
    return IsPointer(id, std::is_pointer<T>());
  }

  template <typename T>
  static inline unsigned long Convert(const T& id)
  {
    return IsIntegral(id, std::is_integral<T>());
  }
  
public:
  static inline unsigned long Self()
  {
    return Convert(pthread_self());
  }
};

} /* net namespace */
} /* util namespace */

#endif
