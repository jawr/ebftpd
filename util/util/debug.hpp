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

inline void DumpBacktrace(std::ostream& out, unsigned skip = 0)
{
  static const size_t maxSize = 100;

  void *array[maxSize];
  
  int size = backtrace(array, maxSize);
  if (size <= 0) return;
  
  boost::scoped_array<char*> symbols(backtrace_symbols(array, size));
  if (!symbols) return;
  
  for (int i = skip; i < size; ++i)
  { 
    std::string symbol(symbols[i]);
    std::string::size_type pos = symbol.find('(');
    std::string path(symbol, 0, pos);
    std::string mangled(symbol, pos + 1, symbol.find(')') - pos - 1);
    std::string offset;

    pos = mangled.find('+');
    if (pos != std::string::npos)
    {
      offset.assign(mangled, pos + 1, mangled.length() - pos);
      mangled.erase(pos);
    }
    
    pos = symbol.find('[');
    std::string address(symbol, pos + 1, symbol.find(']', pos) - pos - 1);

    out << std::left << std::setw(3) << (i - skip + 1);
        
    boost::scoped_array<char> demangled(abi::
        __cxa_demangle(mangled.c_str(), nullptr, nullptr, nullptr)); 

    out << path << " ";
    if (demangled.get()) out << demangled.get() << " ";
    else if (!mangled.empty()) out << mangled << " ";
    if (!offset.empty()) out << "[+" << offset << "] ";
    out << "[" << address << "]" << std::endl;
  }
}

inline bool Demangle(const char* mangled, char* demangled, size_t size)
{
  int status;
  return abi::__cxa_demangle(mangled, demangled, &size, &status) != nullptr;
}

inline std::string Demangle(const char* mangled)
{
  char demangled[124];
  if (Demangle(mangled, demangled, sizeof(demangled))) return std::string(demangled);
  return std::string(mangled);
}

bool DebuggerAttached();

} /* debug namespace */
} /* util namespace */

#endif
