#include <csignal>
#include <cerrno>
#include <stdexcept>
#include <exception>
#include "ftp/task/task.hpp"
#include "signals/signal.hpp"
#include "logs/logs.hpp"
#include "util/debug.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "cfg/error.hpp"
#include "cfg/get.hpp"
#include "util/misc.hpp"

namespace signals
{

std::unique_ptr<Handler> Handler::instance;

const char* SignalName(int signo)
{
  switch (signo)
  {
    case SIGSEGV  : return "SIGSEGV";
    case SIGABRT  : return "SIGABRT";
    case SIGBUS   : return "SIGBUS";
    case SIGILL   : return "SIGILL";
    case SIGFPE   : return "SIGFPE";
    case SIGINT   : return "SIGINT";
    case SIGTERM  : return "SIGTERM";
    case SIGHUP   : return "SIGHUP";
    case SIGQUIT  : return "SIGQUIT";
    default       : return "UNKNOWN";
  }  
}

void Handler::StartThread()
{
  instance.reset(new Handler());
  logs::Debug("Starting signal handling thread..");
  instance->Start();
}

void Handler::StopThread()
{
  logs::Debug("Stopping signal handling thread..");
  instance->Stop();
  instance = nullptr;
}

void Handler::Run()
{
  util::SetProcessTitle("SIGNALS");
  
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
    logs::Debug("Signal received: %1% (%2%)", signo, SignalName(signo));
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
          logs::Error("Failed to load config: %1%", e.Message());
        }
        try
        {
          text::Factory::Initialize();
        }
        catch (const text::TemplateError& e)
        {
          logs::Error("Templates failed to initialise: %1%", e.Message());
        }
        break;
      }
      case SIGTERM  :
      case SIGQUIT  :
      case SIGINT   :
      {
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
  const char* signame = SignalName(signo);
  
  std::stringstream ss;
  ss << "Critical error signal " << signo;
  if (signame) ss << " (" << signame << ")";
  ss << " received, dumping backtrace: " << std::endl;

  util::debug::DumpBacktrace(ss, 2);
  
  std::string line; 
  while (std::getline(ss, line))
  {
    logs::Error(line);
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
    logs::Error(line);
  }
  
  PropogateSignal(SIGABRT);
}

util::Error Initialise()
{
  sigset_t set;
  sigfillset(&set);

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

  if (pthread_sigmask(SIG_BLOCK, &set, nullptr) < 0)
    return util::Error::Failure(errno);  

  return util::Error::Success();
}

} /* signals namespace */
