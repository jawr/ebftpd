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
