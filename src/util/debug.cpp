#include "util/debug.hpp"

namespace util { namespace debug
{

void DumpBacktrace(std::ostream& out, unsigned skip)
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

} /* debug namespace */
} /* util namespace */

#ifdef UTIL_DEBUG_TEST

#include <iostream>
#include <csignal>

using namespace util::debug;
void Handler(int signo)
{
  std::cerr << "Critical error signal " << signo << " received, dumping backtrace: " << std::endl;
  DumpBacktrace(std::cerr, 50);
}

void Foo1()
{
  std::cout << 0 << " " << StackFunctionName<0>() << std::endl;
  std::cout << 1 << " " << StackFunctionName<1>() << std::endl;
  std::cout << 2 << " " << StackFunctionName<2>() << std::endl;
  std::cout << 3 << " " << StackFunctionName<3>() << std::endl;
  std::cout << 4 << " " << StackFunctionName<4>() << std::endl;
  std::cout << 5 << " " << StackFunctionName<5>() << std::endl;
  std::cout << std::endl;
  //DumpBacktrace(std::cerr, 10);
  signal(SIGABRT, Handler);
  abort();
}

void Foo2() { Foo1(); }
void Foo3() { Foo2(); }
void Foo4() { Foo3(); }

int main()
{
  Foo4();
}

#endif
