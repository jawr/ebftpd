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
