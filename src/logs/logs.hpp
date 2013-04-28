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

#ifndef __LOGS_LOGS_HPP
#define __LOGS_LOGS_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include "logs/logger.hpp"
#include "util/format.hpp"
#include "util/string.hpp"
#include "logs/sink.hpp"

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

/*
 * debug, error and database logs have a thread id before the message
 * this id is prefixed with an uppercase character identifying the thread type:
 *
 * P - parent thread
 * L - listener / server
 * S - signals
 * R - database replicator
 * C - client
 * A - asynchronous task
 *
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

inline void Event(const std::string& what, const std::vector<std::pair<std::string, std::string>>& pairs)
{
  extern Logger events;
  events.PushEvent(util::ToUpperCopy(what), pairs);
}

template <typename... Args>
void Security(const std::string& what, const std::string& format, const Args&... args)
{
  extern Logger security;
  std::ostringstream os;
  os << util::ToUpperCopy(what) << ": " << format;
  security.PushEntry("message", util::Format()(os.str(), args...).String());
}

template <typename... Args>
void Debug(const std::string& format, const Args&... args)
{
  extern Logger debug;
  extern std::string ThreadID();
  debug.PushEntry("thread", Brackets('[', ']'), ThreadID(), "message", util::Format()(format, args...).String());
}

template <typename... Args>
void Database(const std::string& format, const Args&... args)
{
  extern Logger db;
  extern std::string ThreadID();
  db.PushEntry("thread", Brackets('[', ']'), ThreadID(), "message", util::Format()(format, args...).String());
}

template <typename... Args>
void Error(const std::string& format, const Args&... args)
{
  extern Logger error;
  extern std::string ThreadID();
  error.PushEntry("thread", Brackets('[', ']'), ThreadID(), "message", util::Format()(format, args...).String());
}

inline void Transfer(const std::string& path, const std::string& direction, 
      const std::string& username, const std::string& groupname, 
      double startTime, long long kBytes, double xfertime, 
      bool okay, const std::string& section)
{
  extern Logger transfer;
  transfer.PushEntry(QuoteOn(), "epoch start", startTime, "direction", direction,
                     "username", username, "groupname", groupname,
                     "size", kBytes, "seconds", xfertime, "okay", okay ? "okay" : "fail",
                     "section", section, "path", path);
}

void InitialisePreConfig();
bool InitialisePostConfig();
void SetThreadIDPrefix(char ch);

}

#endif
