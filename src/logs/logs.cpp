#define __LOGGER_LOGGER_CPP
#include "logs/logs.hpp"

namespace logs
{

util::logger::Logger events;
util::logger::Logger security;
util::logger::Logger sysop;
util::logger::Logger error;
util::logger::Logger debug;
util::logger::Logger db;

void Initialise(const std::string& dataPath)
{
  events.SetPath(dataPath + "/ftpd.log");
  security.SetPath(dataPath + "/access.log");
  sysop.SetPath(dataPath + "/siteop.log");
  error.SetPath(dataPath + "/error.log");
  debug.SetPath(dataPath + "/debug.log");
  db.SetPath(dataPath + "/db.log");
}

} /* logger namespace */

#ifdef LOGGER_LOGGER_TEST

int main()
{
  using namespace logger;
  
  
  logs::Initialise("/tmp");
  logs::debug << "test" << logs::endl;
  logs::security << "test" << logs::endl;
  logs::siteop << "test" << logs::endl;
  logs::error << "test" << logs::endl;
}

#endif
