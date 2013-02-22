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
		
		if (ptrace(PTRACE_ATTACH, ppid, nullptr, nullptr) < 0)
		{
			if (errno == EPERM) _exit(1);
		}
		else
		{
			waitpid(ppid, nullptr, 0);
			ptrace(PTRACE_CONT, ppid, nullptr);
			ptrace(PTRACE_DETACH, ppid, nullptr, nullptr);
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
