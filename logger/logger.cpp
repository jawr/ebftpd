#define __UTIL_LOGGER_CPP
#include "logger.hpp"

namespace logger
{

util::logger::Logger ftpd;
util::logger::Logger access;
util::logger::Logger siteop;
util::logger::Logger error;

void Initialise(const std::string& dataPath)
{
  ftpd.SetPath(dataPath + "/access.log");
  access.SetPath(dataPath + "/access.log");
  siteop.SetPath(dataPath + "/siteop.log");
  error.SetPath(dataPath + "/error.log");
}

} /* logger namespace */
