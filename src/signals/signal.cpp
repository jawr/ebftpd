//    Copyright (C) 2012, 2013 ebftpd team
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

#include <limits>
#include <cstring>
#include <ctime>
#include <csignal>
#include <cerrno>
#include <stdexcept>
#include <exception>
#include <fcntl.h>
#include <execinfo.h>
#include "ftp/task/task.hpp"
#include "signals/signal.hpp"
#include "logs/logs.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "cfg/error.hpp"
#include "cfg/get.hpp"
#include "util/misc.hpp"

namespace signals
{

std::unique_ptr<Handler> Handler::instance;

namespace
{
std::string logPath;
const int maxBacktraceFrames = 100;
}

inline char* IntegerToASCII(int i, char* buf, size_t size)
{
  buf[--size] = '\0';
  char* p1 = buf + size;

  bool negative = false;
  if (i < 0)
  {
    negative = true;
    i = -i;
    --size;
  }
  
  if (size > 0)
  {
    size_t len = 0;
    do
    {
      *--p1 = '0' + (i % 10);
      i /= 10;
      if ( ++len >= size) break;
    }
    while (i != 0);
  }
  
  if (i != 0) return NULL;
  
  if (negative) *--p1 = '-';
  
  char *p2 = buf;
  if (p2 != p1)
  {
    do
    {
      *p2++ = *p1;
    }
    while (*p1++ != '\0');
  }
  
  return p2;
}

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
  logs::SetThreadIDPrefix('S' /* signals */);
  
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
  if (sigaction(SIGSEGV, &sa, nullptr) < 0 || kill(getpid(), signo) < 0)
  {
    _exit(1);
  }
}

void CrashPrint(int efd, int dfd, const char* s)
{
  size_t len = strlen(s);
  if (efd != -1) write(efd, s, len);
  if (dfd != -1) write(dfd, s, len);
}

inline void DumpBacktrace(const char* type, const char* message)
{  
  char pid[std::numeric_limits<pid_t>::digits10 + 2];
  IntegerToASCII(getpid(), pid, sizeof(pid));
  
  char time[std::numeric_limits<time_t>::digits10 + 2];
  IntegerToASCII(std::time(NULL), time, sizeof(time));
  
  char dumpPath[PATH_MAX] = { 0 };
  strncat(dumpPath, logPath.c_str(), sizeof(dumpPath));
  strncat(dumpPath, "/crash.", sizeof(dumpPath));
  strncat(dumpPath, pid, sizeof(dumpPath));
  strncat(dumpPath, ".", sizeof(dumpPath));
  strncat(dumpPath, time, sizeof(dumpPath));
  
  int dfd = open(dumpPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  int efd = STDERR_FILENO;

  CrashPrint(efd, dfd, "Critical error ");
  CrashPrint(efd, dfd, type);
  
  CrashPrint(efd, dfd, " (");
  CrashPrint(efd, dfd, message);
  CrashPrint(efd, dfd, ") received.");
  
  if (dfd >= 0)
  {
    CrashPrint(efd, -1, " Dumping backtrace in file: ");
  }
  else
  {
    CrashPrint(efd, -1, " Unable to create backtrace dump file: ");
  
  }
  
  CrashPrint(efd, -1, dumpPath);
  CrashPrint(efd, dfd, "\n");
  
  if (dfd >= 0)
  {
    void* frames[maxBacktraceFrames];
    
    int numFrames = backtrace(frames, maxBacktraceFrames);
    backtrace_symbols_fd(frames, numFrames, dfd);
    
    close(dfd);
  }
}  

void CrashHandler(int signo)
{
  const char* sigName = SignalName(signo);
  char signoStr[std::numeric_limits<int>::digits10 + 2];
  IntegerToASCII(signo, signoStr, sizeof(signoStr));
  DumpBacktrace(signoStr, sigName);
  PropogateSignal(signo);
}

void TerminateHandler()
{
  static bool rethrown = false;

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
    DumpBacktrace(typeid(e).name(), e.what());
  }
  catch (...)
  {
    DumpBacktrace("unknown exception", "unknown");
  }

  PropogateSignal(SIGABRT);
}

util::Error Initialise(const std::string& logPath_)
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

  logPath = logPath_;

  return util::Error::Success();
}

} /* signals namespace */
