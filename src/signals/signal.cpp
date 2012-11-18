#include <csignal>
#include <cerrno>
#include <sys/ptrace.h>
#include "ftp/listener.hpp"
#include "signals/signal.hpp"
#include "logs/logs.hpp"

namespace signals
{

Handler Handler::instance;

void Handler::StartThread()
{
  instance.Start();
}

void Handler::StopThread()
{
  instance.Stop();
}

void Handler::Run()
{
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGHUP);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGQUIT);
  sigaddset(&mask, SIGTERM);
  
  int signo;
  while (true)
  {
    sigwait(&mask, &signo);
    switch (signo)
    {
      case SIGHUP   :
      {
        try
        {
          LoadConfig();
        }
        catch (const cfg::ConfigError& e)
        {
          logs::error << "Failed to load config: " + e.Message() << logs::endl;
        }
        break;
      }
      case SIGTERM  :
      case SIGQUIT  :
      case SIGINT   :
      {
        logs::debug << "Server interrupted!" << logs::endl;
        ftp::Listener::SetShutdown();
        return;
      }
    }
  }
}

util::Error Initialise()
{
  sigset_t set;
  sigfillset(&set);
  
  // allow interruption inside gdb
  if (ptrace(PTRACE_TRACEME, 0, NULL, 0) < 0 && errno == EPERM)
    sigdelset(&set, SIGINT);

  if (pthread_sigmask(SIG_BLOCK, &set, nullptr) < 0)
    return util::Error::Failure(errno);  

  return util::Error::Success();
}

} /* signals namespace */
