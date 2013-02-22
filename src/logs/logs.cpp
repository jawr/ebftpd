#define __LOGS_LOGS_CPP

#include <fstream>
#include "logs/logs.hpp"
#include "util/path/path.hpp"
#include "cfg/get.hpp"
#include "logs/streamsink.hpp"
#if !defined(EXTERNAL_TOOL)
#include "db/logsink.hpp"
#endif

namespace logs
{

Logger events;
Logger security;
Logger siteop;
Logger error;
Logger debug;
Logger db;

Format Database([](const std::string& message) { db.Write("message", message); });
Format Error([](const std::string& message) { error.Write("message", message); });
Format Debug([](const std::string& message) { debug.Write("message", message); });

void InitialisePreConfig()
{
  error.PushSink(std::make_shared<StreamSink>(Stream(&std::clog, false)));
  debug.PushSink(std::make_shared<StreamSink>(Stream(&std::clog, false)));
}

void InitialiseLog(Logger& logger, const cfg::setting::Log& config)
{
  if (config.Console())
  {
    logger.PushSink(std::make_shared<StreamSink>(Stream(&std::clog, false)));
  }
  if (config.File())
  {
    Stream stream(new std::ofstream(util::path::Join(cfg::Get().Datapath(), config.Filename()).c_str()), true);
    logger.PushSink(std::make_shared<StreamSink>(stream));
  }
#if !defined(EXTERNAL_TOOL)
  if (config.Database()) logger.PushSink(std::make_shared<db::LogSink>(config.Filename()));
#endif
}

void InitialisePostConfig()
{
  error = Logger();
  debug = Logger();
  
  const cfg::Config& config = cfg::Get();
  InitialiseLog(events,config.EventLog()); 
  InitialiseLog(security, config.SecurityLog());
  InitialiseLog(siteop, config.SiteopLog());
  InitialiseLog(error, config.ErrorLog());
  InitialiseLog(debug, config.DebugLog());
  InitialiseLog(db, config.DatabaseLog());
}

void DisableConsole()
{
}

} /* logger namespace */
