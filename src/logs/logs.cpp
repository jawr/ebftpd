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
  events.SetPath(dataPath + "/ftpd.log", true);
  security.SetPath(dataPath + "/access.log", true);
  sysop.SetPath(dataPath + "/siteop.log", true);
  error.SetPath(dataPath + "/error.log", true);
  debug.SetPath(dataPath + "/debug.log", true);
  db.SetPath(dataPath + "/db.log", true);
}

void NoStdout()
{
  events.SetStdoutAlso(false);
  security.SetStdoutAlso(false);
  sysop.SetStdoutAlso(false);
  error.SetStdoutAlso(false);
  debug.SetStdoutAlso(false);
  db.SetStdoutAlso(false);
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
