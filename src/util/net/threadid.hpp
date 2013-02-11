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
