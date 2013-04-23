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

void InitialiseLog(Logger& logger, const cfg::Log& config)
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
    logger.PushSink(std::make_shared<db::LogSink>("log." + config.Name(), 
              config.CollectionSize()));
  }
}

bool InitialisePostConfig()
{
  error = Logger();
  debug = Logger();
  
  const cfg::Config& config = cfg::Get();
  InitialiseLog(db, config.DatabaseLog());

  try
  {
    InitialiseLog(events,config.EventLog()); 
    InitialiseLog(security, config.SecurityLog());
    InitialiseLog(siteop, config.SiteopLog());
    InitialiseLog(error, config.ErrorLog());
    InitialiseLog(debug, config.DebugLog());
    InitialiseLog(transfer, config.TransferLog());
  }
  catch (const db::LogCreationError&)
  {
    Database("Creation of one or more database log collections failed.");
    return false;
  }
  
  return true;
}

std::string ThreadID()
{
  using namespace boost;
  std::string tid = lexical_cast<std::string>(this_thread::get_id());
  if (tid.empty()) tid = ":PARENT:";
  return tid;
}

} /* logger namespace */
