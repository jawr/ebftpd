#include <fstream>
#include "logs/logs.hpp"
#include "util/path/path.hpp"
#include "cfg/get.hpp"
#include "logs/streamsink.hpp"
#ifndef EXTERNAL_TOOL
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

std::shared_ptr<StreamSink> consoleSink;

Format Database([](const std::string& message) { db.Write("message", message); });
Format Error([](const std::string& message) { error.Write("message", message); });
Format Debug([](const std::string& message) { debug.Write("message", message); });

void InitialisePreConfig()
{
  consoleSink = std::make_shared<StreamSink>(Stream(&std::clog, false));
  error.PushSink(consoleSink);
  debug.PushSink(consoleSink);
}

void InitialiseLog(Logger& logger, const cfg::setting::Log& config)
{
  if (config.Console())
  {
    logger.PushSink(consoleSink);
  }
  
  if (config.File())
  {
    Stream stream(new std::ofstream(util::path::Join(cfg::Get().Datapath(), 
              "/logs/" + config.Name() + ".log").c_str()), true);
    logger.PushSink(std::make_shared<StreamSink>(stream));
  }
  
#ifndef EXTERNAL_TOOL
  if (config.Database())
  {
    logger.PushSink(std::make_shared<db::LogSink>("log." + config.Name(), 
              config.CollectionSize()));
  }
#endif
}

bool InitialisePostConfig()
{
  error = Logger();
  debug = Logger();
  
  const cfg::Config& config = cfg::Get();
  InitialiseLog(db, config.DatabaseLog());

#ifndef EXTERNAL_TOOL
  try
  {
#endif
    InitialiseLog(events,config.EventLog()); 
    InitialiseLog(security, config.SecurityLog());
    InitialiseLog(siteop, config.SiteopLog());
    InitialiseLog(error, config.ErrorLog());
    InitialiseLog(debug, config.DebugLog());
#ifndef EXTERNAL_TOOL
  }
  catch (const db::LogCreationError&)
  {
    Database("Creation of one or more database log collections failed.");
    return false;
  }
#endif
  
  return true;
}

} /* logger namespace */
