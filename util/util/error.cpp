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
