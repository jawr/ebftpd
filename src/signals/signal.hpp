#ifndef __SIGNAL__SIGNAL_HPP
#define __SIGNAL__SIGNAL_HPP

#include "util/thread.hpp"
#include "util/error.hpp"

namespace signals
{

class Handler : public util::Thread
{
  void Run();
  
  static Handler instance;
  
public:
  static void StartThread();
  static void StopThread();
};


util::Error Initialise();

} /* signals namespace */

#endif
