#define __LOGGER_LOGGER_CPP
#include "logs/logs.hpp"
#include "fs/path.hpp"

namespace logs
{

util::logger::Logger events;
util::logger::Logger security;
util::logger::Logger sysop;
util::logger::Logger error;
util::logger::Logger debug;
util::logger::Logger db;

void Initialise(const fs::Path& dataPath)
{
  events.SetPath((dataPath & "/ftpd.log").ToString(), true);
  security.SetPath((dataPath & "/access.log").ToString(), true);
  sysop.SetPath((dataPath & "/siteop.log").ToString(), true);
  error.SetPath((dataPath & "/error.log").ToString(), true);
  debug.SetPath((dataPath & "/debug.log").ToString(), true);
  db.SetPath((dataPath & "/db.log").ToString(), true);
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
