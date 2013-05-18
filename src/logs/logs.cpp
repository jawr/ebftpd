//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <cctype>
#include <fstream>
#include <boost/thread/thread.hpp>
#include "logs/logs.hpp"
#include "util/path/path.hpp"
#include "cfg/get.hpp"
#include "logs/streamsink.hpp"
#include "logs/filesink.hpp"
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
Logger transfer;

std::shared_ptr<StreamSink> consoleSink;

void InitialisePreConfig()
{
  consoleSink = std::make_shared<StreamSink>(Stream(&std::clog, false));
  error.PushSink(consoleSink);
  debug.PushSink(consoleSink);
}

bool InitialiseLog(Logger& logger, const cfg::Log& config)
{
  if (config.Console())
  {
    logger.PushSink(consoleSink);
  }
  
  if (config.File())
  {
    logger.PushSink(std::make_shared<FileSink>(util::path::Join(cfg::Get().Datapath(), 
              "logs/" + config.Name() + ".log")));
  }

  if (config.Database())
  {
    try
    {
      logger.PushSink(std::make_shared<db::LogSink>("log." + config.Name(), 
                      config.CollectionSize()));
    }
    catch (const db::LogCreationError&)
    {
      return false;
    }
  }
  
  return true;
}

bool InitialisePostConfig()
{
  error = Logger();
  debug = Logger();
  
  const cfg::Config& config = cfg::Get();
  InitialiseLog(db, config.DatabaseLog());

  bool okay = true;
  okay = InitialiseLog(events,config.EventLog()) && okay;
  okay = InitialiseLog(security, config.SecurityLog()) && okay;
  okay = InitialiseLog(siteop, config.SiteopLog()) && okay;
  okay = InitialiseLog(error, config.ErrorLog()) && okay;
  okay = InitialiseLog(debug, config.DebugLog()) && okay;
  okay = InitialiseLog(transfer, config.TransferLog()) && okay;

  if (!okay)
  {
    Database("Creation of one or more database log collections failed.");
    return false;
  }
  
  return true;
}

namespace
{
static __thread char threadIdPrefix;
}

void SetThreadIDPrefix(char ch)
{
  threadIdPrefix = std::toupper(ch);
}

std::string ThreadID()
{
  using namespace boost;
  std::ostringstream os;
  if (threadIdPrefix == '\0') threadIdPrefix = 'A'; // Asynchronous
  os << threadIdPrefix << std::left << std::setw(14) << this_thread::get_id();
  return os.str();
}

} /* logger namespace */
