#define __LOGGER_LOGGER_CPP
#include "logs/logs.hpp"
#include "util/path/path.hpp"

namespace logs
{

util::logger::Logger events;
util::logger::Logger security;
util::logger::Logger sysop;
util::logger::Logger error;
util::logger::Logger debug;
util::logger::Logger db;

void Initialise(const std::string& logsPath)
{
  events.SetPath(util::path::Append(logsPath, "/ftpd.log"), true);
  security.SetPath(util::path::Append(logsPath, "/access.log"), true);
  sysop.SetPath(util::path::Append(logsPath, "/siteop.log"), true);
  error.SetPath(util::path::Append(logsPath, "/error.log"), true);
  debug.SetPath(util::path::Append(logsPath,"/debug.log"), true);
  db.SetPath(util::path::Append(logsPath, "/db.log"), true);
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
