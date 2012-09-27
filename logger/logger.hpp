#ifndef __LOGGER_LOGGER_HPP
#define __LOGGER_LOGGER_HPP

#include <string>
#include "util/logger.hpp"

namespace logger
{

void Initialise(const std::string& dataPath);

/*
 * log types:
 *
 * ftpd   - for user events, mkdir, rmdir, login, logout 
 * access - bad password, connect from unknown host and other access / security errors
 * siteop - user and group management events
 * error  - any kind of exceptional failure, config parse error, unable to write owner file, etc
 */

#ifndef __LOGGER_LOGGER_CPP
extern util::logger::Logger ftpd;
extern util::logger::Logger access;
extern util::logger::Logger siteop;
extern util::logger::Logger error;
extern util::logger::Logger debug;
#endif

using util::logger::flush;
using util::logger::endl;

}

#endif
