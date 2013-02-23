#ifndef __LOGS_LOGS_HPP
#define __LOGS_LOGS_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include "logs/logger.hpp"
#include "util/format.hpp"
#include "util/string.hpp"

namespace logs
{

void Initialise(const std::string& logsPath);

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

template <typename... Args>
void Siteop(const std::string& admin, const std::string& format, const Args&... args)
{
  extern Logger siteop;
  siteop.PushEntry("admin", Quote('\''), admin, "message", util::Format()(format, args...));
}

template <typename... Args>
void Event(const std::string& what, const Args&... args)
{
  extern Logger events;
  events.PushEntry("event", Tag(), util::ToUpperCopy(what), QuoteOn(), args...);
}

template <typename... Args>
void Security(const std::string& what, const std::string& format, const Args&... args)
{
  extern Logger security;
  std::ostringstream os;
  os << util::ToUpperCopy(what) << ": " << format;
  security.PushEntry("message", util::Format()(os.str(), args...).String());
}

extern Format Database;
extern Format Error;
extern Format Debug;

void InitialisePreConfig();
bool InitialisePostConfig();

}

#endif
