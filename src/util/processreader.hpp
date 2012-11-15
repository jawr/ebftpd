#ifndef __UTIL_PROCESSREADER_HPP
#define __UTIL_PROCESSREADER_HPP

#include <string>
#include <csignal>
#include <vector>
#include <sys/wait.h>
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
  static const time_t defaultTermTimeout = 2;
  static const time_t defaultKillTimeout = 1;

  std::string file;
  char** argv;
  char** env;
  pid_t pid;
  util::Pipe pipe;
  util::InterruptPipe interruptPipe;
  int exitStatus;
  bool eof;

  char getcharBuffer[defaultBufferSize];
  char* getcharBufferPos;
  size_t getcharBufferLen;
  
  void DoParent();
  void DoChild();
  void Open();
  
  size_t Read(char* buffer, size_t size, const util::TimePair* timeout);
  int GetcharBuffered(const util::TimePair* timeout);
  bool Getline(std::string& buffer, const util::TimePair* timeout);
  static char** PrepareArgv(const ArgvType& argv);
  static void FreeArgv(char*** argv);

public:

  ProcessReader();
  ProcessReader(const std::string& file, const ArgvType& argv, const ArgvType& env);
  ~ProcessReader();
    
  void Open(const std::string& file, const ArgvType& argv, const ArgvType& env);
  
  bool Getline(std::string& buffer, const util::TimePair& timeout)
  { return Getline(buffer, &timeout); }
  
  bool Getline(std::string& buffer)
  { return Getline(buffer, nullptr); }
 
  bool Close();  
  bool Close(const util::TimePair& timeout);
  int ExitStatus() const { return exitStatus; }
  bool Kill(int signo, const util::TimePair& timeout);
  bool Kill(const util::TimePair& timeout)
  { return Kill(SIGTERM, timeout); }
  
  void Interrupt();
};

} /* util namespace */

#endif
