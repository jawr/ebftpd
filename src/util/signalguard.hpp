#ifndef __UTIL_SIGNAL_HPP
#define __UTIl_SIGNAL_HPP

#include <csignal>
#include <pthread.h>

namespace util
{

class SignalGuard
{
private:
  sigset_t sig_mask;
  int sigNo;
  bool alreadyPending;
  bool needUnblock;

public:
  SignalGuard(int sigNo) :
    sigNo(sigNo)
  {
    sigset_t pending;
    sigpending(&pending);
    alreadyPending = sigismember(&pending, sigNo);

    if (!alreadyPending)
    {
      sigemptyset(&sig_mask);
      sigaddset(&sig_mask, sigNo);
      sigset_t blocked;
      pthread_sigmask(SIG_BLOCK, &sig_mask, &blocked);
      needUnblock = sigismember(&blocked, sigNo);
    }
  }

  ~SignalGuard()
  {
    if (!alreadyPending)
    {
      sigset_t pending;
      sigpending(&pending);

      if (sigismember(&pending, sigNo))
        sigwaitinfo(&sig_mask, 0);

      if (needUnblock) pthread_sigmask(SIG_UNBLOCK, &sig_mask, 0);
    }
  }
};

} /* util namespace */

#endif

