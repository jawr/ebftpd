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

#ifdef LOGGER_TEST
#include <fstream>
#include "logs/logger.hpp"
#include "logs/streamsink.hpp"
//#include "db/logsink.hpp"

int main()
{
  using namespace logs;
  
  Logger logger;
  logger.PushSink(std::make_shared<StreamSink>(Stream(&std::clog, false)));
  
  std::ofstream* log = new std::ofstream("/tmp/log.log");
  logger.PushSink(std::make_shared<StreamSink>(Stream(log, true)));
//  logger.PushSink(std::make_shared<db::LogSink>("somecol"));
  
  logger.Write("message", QuoteOn('\''), "this is a message", "kbytes", 1243432, QuoteOff(), "size", 100.20);
}
#endif
