#ifndef __LOGGER_LOGGER_HPP
#define __LOGGER_LOGGER_HPP

#include <ostream>
#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/tss.hpp>

namespace logger
{

// adapted from:
//
// http://stackoverflow.com/questions/5528207/thread-safe-streams-and-stream-manipulators
//
// std::flush and std::endl are incompatible with the stream,
// the provided logger::flush and logger::endl must be used instead.
//
// redirects automatically to std::clog if unable to open / write to log file
// 

class Logger : boost::noncopyable
{
  std::string path;
  boost::mutex outMutex;
  std::ostream* out;
  boost::thread_specific_ptr<std::ostringstream> buffer;

  std::string Timestamp();
  Logger& Flush(bool newLine);
  
public:
  Logger() : out(0) { }
  Logger(const std::string& path) : path(path), out(0) { }
  ~Logger();

  void SetPath(const std::string& path);
  
  template <typename T> Logger& operator<<(T data);
  Logger& operator<<(std::ostream& (*pf)(std::ostream&));
  Logger& operator<<(Logger& (*pf)(Logger&));
  
  friend Logger& flush(Logger& logger);
  friend Logger& endl(Logger& logger);
};

Logger& flush(Logger& logger);
Logger& endl(Logger& logger);

void Initialise();

#ifndef __LOGGER_LOGGER_CPP

extern Logger ftpd;
extern Logger access;
extern Logger siteop;
extern Logger error;

#endif
}

#endif
