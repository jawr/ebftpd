#ifndef __LOGGER_LOGGER_HPP
#define __LOGGER_LOGGER_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include <boost/algorithm/string/case_conv.hpp>
#include "logs/logger.hpp"
#include "util/format.hpp"

namespace logs
{

void Initialise(const std::string& logsPath);

void DisableConsole();

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

struct Format : public util::Format
{
  Format& operator=(Format&&) = delete;
  Format& operator=(const Format&) = delete;
  Format(Format&&) = delete;
  Format(const Format&) = delete;
  
  Format(const util::Format::OutputFunction& output) :
    util::Format(output)
  { }
};
 

inline void Log(Logger& logger, std::ostringstream& os)
{
  logger.Write("message", os.str());
}

template <typename T, typename... Args>
void Log(Logger& logger, std::ostringstream& os, const T& arg, const Args&... args)
{
  os << " \"" << arg << "\"";
  Log(logger, os, args...);
}

template <typename... Args>
void Siteop(const std::string& who, const std::string& what, const Args&... args)
{
  extern Logger siteop;
  std::ostringstream os;
  os << '[' << std::left << std::setw(15) << who << "] " << boost::to_upper_copy(what) << ":";
  Log(siteop, os, args...);
}

template <typename... Args>
void Event(const std::string& what, const Args&... args)
{
  extern Logger events;
  std::ostringstream os;
  os << boost::to_upper_copy(what) << ":";
  Log(events, os, args...);
}

template <typename... Args>
void Security(const std::string& what, const std::string& format, const Args&... args)
{
  extern Logger security;
  std::ostringstream os;
  os << boost::to_upper_copy(what) << ": " << format;
  security.Write("message", util::Format()(os.str(), args...).String());
}

#ifndef __LOGS_LOGS_CPP
extern Format Database;
extern Format Error;
extern Format Debug;
#endif

void InitialisePreConfig();
void InitialisePostConfig();

}

#endif
