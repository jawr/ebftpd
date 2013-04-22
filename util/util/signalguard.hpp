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

