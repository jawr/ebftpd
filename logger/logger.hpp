#ifndef __LOGGER_LOGGER_HPP
#define __LOGGER_LOGGER_HPP

#include <string>
#include "util/logger.hpp"

namespace logger
{

void Initialise(const std::string& dataPath);

#ifndef __UTIL_LOGGER_CPP
extern util::logger::Logger ftpd;
extern util::logger::Logger access;
extern util::logger::Logger siteop;
extern util::logger::Logger error;
#endif

using util::logger::flush;
using util::logger::endl;

}

#endif
