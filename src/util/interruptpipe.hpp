#ifndef __UTIL_INTERRUPT_PIPE_HPP
#define __UTIL_INTERRUPT_PIPE_HPP

#include "util/pipe.hpp"

namespace util
{

class InterruptPipe : public Pipe
{
public:
  void Interrupt()
  {
    (void) write(WriteFd(), "1", 1);
  }
};

} /* util namespace */

#endif
