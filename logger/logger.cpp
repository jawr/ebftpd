#define __LOGGER_LOGGER_CPP
#include "logger/logger.hpp"

namespace logger
{

util::logger::Logger ftpd;
util::logger::Logger access;
util::logger::Logger siteop;
util::logger::Logger error;
util::logger::Logger debug;

void Initialise(const std::string& dataPath)
{
  ftpd.SetPath(dataPath + "/ftpd.log");
  access.SetPath(dataPath + "/access.log");
  siteop.SetPath(dataPath + "/siteop.log");
  error.SetPath(dataPath + "/error.log");
  debug.SetPath(dataPath + "/debug.log");
}

} /* logger namespace */

#ifdef LOGGER_LOGGER_TEST

int main()
{
  using namespace logger;
  
  
  logger::Initialise("/tmp");
  logger::ftpd << "test" << logger::endl;
  logger::access << "test" << logger::endl;
  logger::siteop << "test" << logger::endl;
  logger::error << "test" << logger::endl;
}

#endif
