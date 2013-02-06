#ifndef __UTIL_KEYBASE_HPP
#define __UTIL_KEYBASE_HPP

namespace util
{

class KeyBase
{
  KeyBase& operator=(KeyBase&&) = delete;
  KeyBase& operator=(const KeyBase&) = delete;
  
  KeyBase(KeyBase&&) = delete;
  KeyBase(const KeyBase&&) = delete;

protected:
  KeyBase() { }
};

} /* util namespace */

#endif
