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

void Initialise(const fs::Path& logsPath)
{
  events.SetPath((logsPath & "/ftpd.log").ToString(), true);
  security.SetPath((logsPath & "/access.log").ToString(), true);
  sysop.SetPath((logsPath & "/siteop.log").ToString(), true);
  error.SetPath((logsPath & "/error.log").ToString(), true);
  debug.SetPath((logsPath & "/debug.log").ToString(), true);
  db.SetPath((logsPath & "/db.log").ToString(), true);
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
