#ifndef __LOGGER_LOGGER_HPP
#define __LOGGER_LOGGER_HPP

#include <string>
#include "util/logger.hpp"

namespace fs
{
class Path;
}

namespace logs
{

void Initialise(const fs::Path& dataPath);

void NoStdout();

/*
 * log types:
 *
 * ftpd   - for user events, mkdir, rmdir, login, logout 
 * access - bad password, connect from unknown host and other access / security errors
 * siteop - user and group management events
 * error  - any kind of exceptional failure, config parse error, unable to write owner file, etc
 */

#ifndef __LOGGER_LOGGER_CPP
extern util::logger::Logger events;
extern util::logger::Logger security;
extern util::logger::Logger sysop;
extern util::logger::Logger error;
extern util::logger::Logger debug;
extern util::logger::Logger db;
#endif

using util::logger::flush;
using util::logger::endl;

}

#endif
