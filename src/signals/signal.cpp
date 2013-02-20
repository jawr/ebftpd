#include <csignal>
#include <cerrno>
#include <stdexcept>
#include <exception>
#include <sys/ptrace.h>

#include "ftp/task/task.hpp"
#include "signals/signal.hpp"
#include "logs/logs.hpp"
#include "util/debug.hpp"

#include "text/error.hpp"
#include "text/factory.hpp"
#include "cfg/error.hpp"
#include "cfg/get.hpp"

namespace signals
{

Handler Handler::instance;

void Handler::StartThread()
{
  logs::debug << "Starting signal handling thread.." << logs::endl;
  instance.Start();
}

void Handler::StopThread()
{
  logs::debug << "Stopping signal handling thread.." << logs::endl;
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


void CrashHandler(int signo)
{
  const char* signame = nullptr;
  
  switch (signo)
  {
    case SIGSEGV  : signame = "SIGSEGV"; break;
    case SIGABRT  : signame = "SIGABRT"; break;
    case SIGBUS   : signame = "SIGBUS"; break;
    case SIGILL   : signame = "SIGILL"; break;
    case SIGFPE   : signame = "SIGFPE"; break;
  }
  
  std::stringstream ss;
  ss << "Critical error signal " << signo;
  if (signame) ss << " (" << signame << ")";
  ss << " received, dumping backtrace: " << std::endl;

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

util::Error Initialise()
{
  sigset_t set;
  sigfillset(&set);

  // allow interruption inside gdb
  int ret = ptrace(PT_TRACE_ME, 0, nullptr, 0);
  if (ret < 0 && errno == EPERM)
  {
    sigdelset(&set, SIGINT);
  }
  else
  {
    sigdelset(&set, SIGSEGV);
    sigdelset(&set, SIGABRT);
    sigdelset(&set, SIGBUS);
    sigdelset(&set, SIGILL);
    sigdelset(&set, SIGFPE);
    
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = 0;
    sa.sa_handler = CrashHandler;
    if (sigaction(SIGSEGV, &sa, nullptr) < 0)
      return util::Error::Failure(errno);
    if (sigaction(SIGBUS, &sa, nullptr) < 0)
      return util::Error::Failure(errno);
    if (sigaction(SIGILL, &sa, nullptr) < 0)
      return util::Error::Failure(errno);
    if (sigaction(SIGFPE, &sa, nullptr) < 0)
      return util::Error::Failure(errno);

   std::set_terminate(TerminateHandler);
 }

  if (pthread_sigmask(SIG_BLOCK, &set, nullptr) < 0)
    return util::Error::Failure(errno);  

  return util::Error::Success();
}

} /* signals namespace */
