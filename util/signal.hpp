#ifndef __SIGNAL_HPP
#define __SIGNAL_HPP

#include <iostream>
#include <csignal>
#include "verify.hpp"
#include <pthread.h>

namespace util
{
  namespace signal
  {
    class scope_block
    {
    private:
      sigset_t sig_mask;
      int sig_no;
      bool already_pending;
      bool need_unblock;

    public:
      scope_block(int sig_no) :
        sig_no(sig_no)
      {
        sigset_t pending;
        verify(!sigpending(&pending));
        already_pending = sigismember(&pending, sig_no);

        if (!already_pending)
        {
          sigemptyset(&sig_mask);
          sigaddset(&sig_mask, sig_no);
          sigset_t blocked;
          verify(!pthread_sigmask(SIG_BLOCK, &sig_mask, &blocked));
          need_unblock = !sigismember(&blocked, sig_no);
        }
      }

      ~scope_block()
      {
        if (!already_pending)
        {
          sigset_t pending;
          verify(!sigpending(&pending));

          if (sigismember(&pending, sig_no))
          {
            sigwaitinfo(&sig_mask, 0);
          }

          if (need_unblock)
          {
            verify(!pthread_sigmask(SIG_UNBLOCK, &sig_mask, 0));
          }
        }
      }
    };
  }
}

#endif

