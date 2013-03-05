#ifndef __PLUGIN_UTIL_HPP
#define __PLUGIN_UTIL_HPP

#include <stdexcept>

namespace plugin
{

struct NoUnlocking
{
  static void Unlock() { }
  static void Lock() { }
};

} /* script namespace */

#endif