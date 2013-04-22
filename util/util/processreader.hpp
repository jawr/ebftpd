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

#ifndef __UTIL_PROCESSREADER_HPP
#define __UTIL_PROCESSREADER_HPP

#include <memory>
#include <string>
#include <csignal>
#include <vector>
#include <sys/wait.h>
#include <boost/thread/mutex.hpp>
#include "util/error.hpp"
#include "util/pipe.hpp"
#include "util/interruptpipe.hpp"
#include "util/timepair.hpp"

namespace util
{

class ProcessReader
{
public:
  typedef std::vector<std::string> ArgvType;

private:
  static const size_t defaultBufferSize = BUFSIZ;
  static const util::TimePair defaultTermTimeout;
  static const util::TimePair defaultKillTimeout;

  std::string file;
  ArgvType argv;
  ArgvType env;
  pid_t pid;
  std::unique_ptr<util::Pipe> pipe;
  std::unique_ptr<util::InterruptPipe> interruptPipe;
  boost::mutex interruptMutex;
  int exitStatus;
  bool eof;

  char getcharBuffer[defaultBufferSize];
  char* getcharBufferPos;
  size_t getcharBufferLen;
  
  void Open();
  
  size_t Read(char* buffer, size_t size, const util::TimePair* timeout);
  int GetcharBuffered(const util::TimePair* timeout);
  bool Getline(std::string& buffer, const util::TimePair* timeout);
  static char** PrepareArgv(const ArgvType& argv);

public:

  ProcessReader();
  ProcessReader(const std::string& file, const ArgvType& argv, const ArgvType& env);
  ~ProcessReader();
    
  void Open(const std::string& file, const ArgvType& argv, const ArgvType& env);
  
  bool Getline(std::string& buffer, const util::TimePair& timeout)
  { return Getline(buffer, &timeout); }
  
  bool Getline(std::string& buffer)
  { return Getline(buffer, nullptr); }
 
  bool Close(bool nohang = true);  
  bool Close(const util::TimePair& timeout);
  int ExitStatus() const { return exitStatus; }
  bool Kill(int signo, const util::TimePair& timeout);
  bool Kill(const util::TimePair& timeout)
  { return Kill(SIGTERM, timeout); }
  
  void Interrupt();
};

} /* util namespace */

#endif
