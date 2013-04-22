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

#include <unistd.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include "util/debug.hpp"

namespace util { namespace debug
{

bool DebuggerAttached()
{
  pid_t pid = fork();
  if (pid < 0) return false;
  if (pid == 0) /* child */
  {
    pid_t ppid = getppid();
    
    if (ptrace(PT_ATTACH, ppid, 0, 0) < 0)
    {
      if (errno == EPERM) _exit(1);
    }
    else
    {
      waitpid(ppid, 0, 0);
      ptrace(PT_CONTINUE, ppid, 0, 0);
      ptrace(PT_DETACH, ppid, 0, 0);
    }
    
    _exit(0);
  }
  else /* parent */
  {
    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status) != 0;
  }
}

} /* debug namespace */
} /* util namespace */
