#include <cassert>
#include <iomanip>
#include <fstream>
#include <ctime>
#include "logger.hpp"

namespace util { namespace logger
{

Logger::Logger() : out(&std::clog) { }
Logger::Logger(const std::string& path) : path(path), out(&std::clog) { }


void Logger::SetPath(const std::string& path)
{
  this->path = path;
}

Logger::~Logger()
{
  if (out != &std::clog) delete out;
}

//template <typename T>
//Logger& Logger::operator<<(T data)
//{
//  if (!buffer.get()) buffer.reset(new std::ostringstream);
//  std::ostringstream* oss = buffer.get();
//  (*oss) << data;
//  return *this;
//}

Logger& Logger::operator<<(std::ostream& (*pf)(std::ostream&))
{
  if (!buffer.get()) buffer.reset(new std::ostringstream);
  std::ostringstream* oss = buffer.get();
  (*oss) << pf;
  return *this;
}  


Logger& Logger::operator<<(Logger& (*pf)(Logger&))
{
  return pf(*this);
}

std::string Logger::Timestamp()
{
  time_t now = time(NULL);
  char buf[26];
  strftime(buf,sizeof(buf),"%a %b %d %T %Y", localtime(&now));
  return buf;
}

// this could be improved by moving printer out
// into a worker thread
Logger& Logger::Flush(bool newLine)
{
  std::string timestamp = Timestamp();
  
  if (!buffer.get()) buffer.reset(new std::ostringstream);
  std::ostringstream* oss = buffer.get();
  
  {
    boost::lock_guard<boost::mutex> lock(outMutex);
    if (out == &std::clog && !path.empty()) out = 0;
    if (!out)
    {
      std::ofstream* fout(new std::ofstream(path.c_str(), std::ios::app));
      if (*fout) out = fout;
      else
      {
        delete fout;
        fout = 0;
        out = &std::clog;
      }
    }

    std::istringstream iss(oss->str());
    std::string line;
    bool firstLine = true;
    while (std::getline(iss, line))
    {
      if (!firstLine) (*out) << "\n";
      else firstLine = false;
      (*out) << timestamp << " " << line;
      #ifdef DEBUG
      //std::clog << timestamp << " " << line;
      #endif
    }

    (*out) << (newLine ? "\n" : "") << std::flush;

    #ifdef DEBUG
    //std::clog << (newLine ? "\n" : "") << std::flush;
    #endif
  }
  
  oss->str("");
  return *this;
}


Logger& flush(Logger& logger)
{
  return logger.Flush(false);
}

Logger& endl(Logger& logger)
{
  return logger.Flush(true);
}

} /* logger namespace */
} /* util namespace */

#ifdef LOGGER_TEST

using namesapce util;

#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

logger::Logger test;

void Test()
{
  for (int i = 0; i < 1000; ++i)
  {
    text << i << logger::endl;
  }
}

int main()
{
  
  test.SetPath("/home/bioboy/ftpd/tmp/logs/test.log");
  text << "test" << logger::endl;
  text << std::fixed << std::setprecision(0) << 69.59 << logger::endl;
  
  boost::thread t1(&Test);
  boost::thread t2(&Test);
  
  t1.join();
  t2.join();
}

#endif
