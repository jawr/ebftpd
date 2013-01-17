#ifndef __UTIL_DEBUG_HPP
#define __UTIL_DEBUG_HPP

#include <execinfo.h>
#include <boost/scoped_array.hpp>
#include <cstdio>
#include <string>
#include <cstring>
#include <cxxabi.h>
#include <iomanip>
#include <limits.h>

namespace util { namespace debug
{

template <unsigned int level>
inline std::string StackFunctionName()
{
  const static unsigned int index = level + 1;
  void  *array[level + 2];

  int  size = backtrace(array, level + 2);
  if (size < level + 2) return "Unknown";
  
  boost::scoped_array<char*> symbols(backtrace_symbols(array, size));
  if (!symbols) return "Unknown";
  
  char mangled[strlen(symbols[index]) + 1];
  if (sscanf(symbols[index], "%*[^(](%[^+)]+", mangled) != 1) return "Unknown";
  
  std::string function;
  boost::scoped_array<char> demangled(abi::__cxa_demangle(mangled, nullptr, nullptr, nullptr)); 
  if (demangled.get()) function.assign(demangled.get());
  else function.assign(mangled);
  
  std::string::size_type pos = function.find_first_of('(');
  if (pos != std::string::npos) function.erase(pos);
  
  return function;
}

void DumpBacktrace(std::ostream& out, unsigned skip = 0);

} /* debug namespace */
} /* util namespace */

#endif
