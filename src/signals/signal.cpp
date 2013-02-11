#include <csignal>
#include <cerrno>
#include <stdexcept>
#include <exception>

#if !defined(__CYGWIN__)
# include <sys/ptrace.h>
#endif

#include "ftp/task/task.hpp"
#include "signals/signal.hpp"
#include "logs/logs.hpp"

#if !defined(__CYGWIN__)
# include "util/debug.hpp"
#endif

#include "text/error.hpp"
#include "text/factory.hpp"
#include "cfg/error.hpp"
#include "acl/replicator.hpp"
#include "cfg/get.hpp"

#if !defined(__CYGWIN__)
#  define USE_PTRACE
#  if !defined(PTRACE_TRACEME)
#    if defined(PTRACE_TRACE_ME)
#      define PTRACE_TRACEME PTRACE_TRACE_ME
#    else
#      undef USE_PTRACE
#    endif
#  endif
#endif

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
  sigaddset(&mask, SIGALRM);
  
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
          cfg::UpdateShared(cfg::Config::Load());
        }
        catch (const cfg::ConfigError& e)
        {
          logs::error << "Failed to load config: " + e.Message() << logs::endl;
        }
        try
        {
          text::Factory::Initialize();
        }
        catch (const text::TemplateError& e)
        {
          logs::error << "Templates failed to initialise: " << e.Message() << logs::endl;
        }
        break;
      }
      case SIGTERM  :
      case SIGQUIT  :
      case SIGINT   :
      {
        logs::debug << "Server interrupted!" << logs::endl;
        std::make_shared<ftp::task::Exit>()->Push();
        return;
      }
      case SIGALRM  :
      {
        acl::Replicator::Replicate();
      }
    }
  }
}

void PropogateSignal(int signo)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_flags = 0;
  sa.sa_handler = SIG_DFL;
  if (sigaction(SIGSEGV, &sa, nullptr) < 0 ||
      kill(getpid(), signo) < 0)
  {
    _exit(1);
  }
}


#if !defined(__CYGWIN__)
void CrashHandler(int signo)
{
  std::stringstream ss;
  ss << "Critical error signal " << signo << " received, dumping backtrace: " << std::endl;

  util::debug::DumpBacktrace(ss, 2);
  
  std::string line; 
  while (std::getline(ss, line))
  {
    logs::error << line << logs::endl;
  }
  
  PropogateSignal(signo);
}

void TerminateHandler()
{
  static bool rethrown = false;
  std::stringstream ss;
  try
  {
    if (!rethrown)
    {
      rethrown = true;
      throw;
    }
  }
  catch (const std::exception& e)
  {
    ss << "Unhandled ";
    char exceptionType[1024];
    if (util::debug::Demangle(typeid(e).name(), exceptionType, sizeof(exceptionType)))
      ss << exceptionType;
    else
      ss << typeid(e).name();
    ss << " (" << e.what() << ") exception, dumping backtrace: " << std::endl;
  }
  catch (...)
  {
    ss << "Unhandled exception, dumping backtrace: " << std::endl;
  }
  
  util::debug::DumpBacktrace(ss, 2);
  
  std::string line; 
  while (std::getline(ss, line))
  {
    logs::error << line << logs::endl;
  }
  
  PropogateSignal(SIGABRT);
}

#endif

util::Error Initialise()
{
  sigset_t set;
  sigfillset(&set);

#if !defined(__CYGWIN__)
  sigdelset(&set, SIGSEGV);
  sigdelset(&set, SIGABRT);
  
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_flags = 0;
  sa.sa_handler = CrashHandler;
  if (sigaction(SIGSEGV, &sa, nullptr) < 0)
    return util::Error::Failure(errno);

#if defined(USE_PTRACE)
  // allow interruption inside gdb
  if (ptrace(PTRACE_TRACEME, 0, nullptr, 0) < 0 && errno == EPERM)
    sigdelset(&set, SIGINT);
#endif

 std::set_terminate(TerminateHandler);
#endif

  if (pthread_sigmask(SIG_BLOCK, &set, nullptr) < 0)
    return util::Error::Failure(errno);  

  return util::Error::Success();
}

} /* signals namespace */
