#ifndef __LOGGER_LOGGER_HPP
#define __LOGGER_LOGGER_HPP

#include <string>
#include <boost/algorithm/string/case_conv.hpp>
#include "util/logger.hpp"
#include "util/format.hpp"

namespace logs
{

void Initialise(const std::string& logsPath);

void NoStdout();

/*
 * log types:
 *
 * events   - for user events, mkdir, rmdir, login, logout 
 * security - bad password, connect from unknown host and other access / security errors
 * siteop   - user and group management events, other siteop tasks / changes
 * error    - any kind of exceptional failure, config parse error, etc
 * db       - any database related event or failure
 * debug    - miscellaneous debugging output
 */

#ifndef __LOGGER_LOGGER_CPP
extern util::logger::Logger security;
extern util::logger::Logger error;
extern util::logger::Logger debug;
extern util::logger::Logger db;
extern util::logger::Logger events;
extern util::logger::Logger siteop;
#endif

using util::logger::flush;
using util::logger::endl;

inline void Log(util::logger::Logger& logger)
{
  logger << endl;
}

template <typename T, typename... Args>
void Log(util::logger::Logger& logger, const T& arg, const Args&... args)
{
  logger << " \"" << arg << "\"";
  Log(logger, args...);
}

template <typename... Args>
void Siteop(const std::string& who, const std::string& what, const Args&... args)
{
  extern util::logger::Logger siteop;
  siteop << '[' << std::left << std::setw(15) << who << "] " << boost::to_upper_copy(what) << ":";
  Log(siteop, args...);
}

template <typename... Args>
void Event(const std::string& what, const Args&... args)
{
  extern util::logger::Logger events;
  events << boost::to_upper_copy(what) << ":";
  Log(events, args...);
}

template <typename... Args>
void Security(const std::string& what, const std::string& format, const Args&... args)
{
  extern util::logger::Logger security;
  std::ostringstream os;
  os << boost::to_upper_copy(what) << ": " << format;
  security << util::Format()(os.str(), args...) << endl;
}

}

#endif
