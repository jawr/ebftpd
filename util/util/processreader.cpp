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

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <boost/thread/thread.hpp>
#include "util/processreader.hpp"

namespace util
{

const util::TimePair ProcessReader::defaultTermTimeout(2);
const util::TimePair ProcessReader::defaultKillTimeout(1);

ProcessReader::~ProcessReader()
{
  if (pid != -1)
  {
    try
    {
      if (!Kill(defaultTermTimeout))
        Kill(SIGKILL, defaultKillTimeout);
    }
    catch (const util::SystemError&)
    {
      // ouch! ignore exception in dtor.
      // no logging facilities in util namespace
      // possibly move this class out?
    }
  }
}

void ProcessReader::Open()
{
  if (pid != -1) throw std::logic_error("Must call Close before calling Open again");
  
  {
    boost::lock_guard<boost::mutex> lock(interruptMutex);
    interruptPipe.reset(new util::InterruptPipe());
  }

  pipe.reset(new util::Pipe());
  util::Pipe errorPipe;
  
  if (fcntl(errorPipe.WriteFd(), F_SETFD, fcntl(errorPipe.WriteFd(), F_GETFD) | FD_CLOEXEC) < 0)
   throw util::SystemError(errno);
  
  pid = fork();
  if (pid < 0) throw util::SystemError(errno);
  
  if (pid)
  {
    pipe->CloseWrite();
    errorPipe.CloseWrite();
    int error;
    ssize_t len;
    while ((len = read(errorPipe.ReadFd(), &error, sizeof(error))) < 0)
      if (errno != EINTR && errno != EAGAIN) break;
    if (len > 0)
    {
      Close(false);
      throw util::SystemError(error);
    }
  }
  else
  {
    errorPipe.CloseRead();
    pipe->CloseRead();
    if (dup2(pipe->WriteFd(), fileno(stdout)) < 0) return;

    execve(file.c_str(), PrepareArgv(argv), PrepareArgv(env));
    pipe->CloseWrite();

    int error = errno;
    while (write(errorPipe.WriteFd(), &error, sizeof(error)) < 0)
      if (errno != EINTR && errno != EAGAIN) break;
    _exit(1);
  }
}

size_t ProcessReader::Read(char* buffer, size_t size, const util::TimePair* timeout)
{
  struct pollfd fds[2];

  fds[0].fd = pipe->ReadFd();
  fds[0].events = POLLIN;

  fds[1].fd = interruptPipe->ReadFd();
  fds[1].events = POLLIN;

  int pollTimeout = timeout ? (timeout->Seconds() * 1000) + (timeout->Microseconds() / 1000) : -1;
  while (true)
  {
    fds[0].revents = 0;    
    fds[1].revents = 0;
    
    int n = poll(fds, 2, pollTimeout);
    if (!n) throw util::SystemError(ETIMEDOUT);
    if (n < 0)
    {
      if (errno == EINTR) continue;
      throw util::SystemError(errno);
    }

    if (fds[1].revents & POLLIN)
    {
      interruptPipe->Acknowledge();
      boost::this_thread::interruption_point();
    }

    if (fds[0].revents & POLLIN)
    {
      ssize_t len = read(pipe->ReadFd(), buffer, size);
      if (len < 0)
      {
        if (errno == EINTR) continue;
        throw util::SystemError(errno);
      }
      return len;
    }
    
    return 0;
  }
}

int ProcessReader::GetcharBuffered(const util::TimePair* timeout)
{
  if (!getcharBufferLen)
  {
    getcharBufferLen = Read(getcharBuffer, sizeof(getcharBuffer), timeout);
    if (!getcharBufferLen) return -1;
    getcharBufferPos = getcharBuffer;
  }
  
  --getcharBufferLen;
  return *getcharBufferPos++;
}

bool ProcessReader::Getline(std::string& buffer, const util::TimePair* timeout)
{
  if (eof) return false;
  
  buffer.clear();
  buffer.reserve(defaultBufferSize);
  
  int ch;
  do
  {
    ch = GetcharBuffered(timeout);
    if (ch == -1) 
    {
      eof = true;
      if (buffer.empty()) return false;
      break;
    }

    if (ch != '\r' && ch != '\n')
    {
      buffer += ch;
    }
  }
  while (ch != '\n');
  return true;
}

char** ProcessReader::PrepareArgv(const ArgvType& argv)
{
  char** a = new char*[argv.size() + 1];
  for (size_t i = 0; i < argv.size(); ++i)
  {
    const std::string& s = argv[i];
    a[i] = new char[s.size() + 1];
    s.copy(a[i], s.size());
    a[i][s.size()] = '\0';
  }
  a[argv.size()] = nullptr;
  return a;
}

ProcessReader::ProcessReader() : 
  pid(-1), exitStatus(-1), eof(false),
  getcharBufferPos(nullptr), getcharBufferLen(0)
{
}

ProcessReader::ProcessReader(const std::string& file, 
    const ArgvType& argv, const ArgvType& env) : 
  file(file), argv(argv), env(env), pid(-1), 
  exitStatus(-1), eof(false), getcharBufferPos(nullptr), 
  getcharBufferLen(0)
{
  Open();
}
  
void ProcessReader::Open(const std::string& file, 
    const ArgvType& argv, const ArgvType& env)
{ 
  this->file.assign(file);
  this->argv.assign(argv.begin(), argv.end());
  this->env.assign(env.begin(), env.end());
  Open();
}

bool ProcessReader::Close(bool nohang)
{
  {
    boost::lock_guard<boost::mutex> lock(interruptMutex);
    interruptPipe = nullptr;
  }

  pipe = nullptr;  
  eof = false;
  getcharBufferPos = getcharBuffer;
  getcharBufferLen = 0;
  
  if (pid == -1) return true;
  while (true)
  {
    int result = waitpid(pid, &exitStatus, nohang ? WNOHANG : 0);
    if (!result) return false;
    if (result < 0)
    {
      if (errno == ECHILD) break;
      if (errno == EINTR) continue;
      pid = -1;
      throw util::SystemError(errno);
    }
    break;
  }
  
  pid = -1;
  return true;
}

bool ProcessReader::Close(const util::TimePair& timeout)
{
  suseconds_t toMicroseconds = (timeout.Seconds() * 1000000) + timeout.Microseconds();
  while (toMicroseconds > 0)
  {
    if (Close()) return true;
    time_t sleepTime = toMicroseconds < 10000 ? toMicroseconds : 10000;
    usleep(sleepTime);
    toMicroseconds -= sleepTime;
  }
  return false;
}

bool ProcessReader::Kill(int signo, const util::TimePair& timeout)
{
  int result = kill(pid, signo);
  if (result < 0)
  {
    if (errno == ESRCH) return Close();
    throw util::SystemError(errno);
  }
   
  suseconds_t toMicroseconds = (timeout.Seconds() * 1000000) + timeout.Microseconds();
  while (toMicroseconds > 0)
  {
    if (Close()) return true;
    time_t sleepTime = toMicroseconds < 10000 ? toMicroseconds : 10000;
    usleep(sleepTime);
    toMicroseconds -= sleepTime;
  }
  
  return false;
}

void ProcessReader::Interrupt()
{
  boost::lock_guard<boost::mutex> lock(interruptMutex);
  if (interruptPipe)
  {
    (void) write(interruptPipe->WriteFd(), "", 1);
  }
}

} /* util namespace */
