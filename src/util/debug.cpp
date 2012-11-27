#include "util/debug.hpp"

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
