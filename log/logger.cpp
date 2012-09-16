#undef __GNUC__
#define __LOGGER_LOGGER_CPP
#include <cassert>
#include <iomanip>
#include <fstream>
#include <ctime>
#include "logger.hpp"

namespace logger
{

Logger ftpd;
Logger access;
Logger siteop;
Logger error;

void Logger::SetPath(const std::string& path)
{
  this->path = path;
}

Logger::~Logger()
{
  if (out != &std::clog) delete out;
}

template <typename T>
Logger& Logger::operator<<(T data)
{
  if (!buffer.get()) buffer.reset(new std::ostringstream);
  std::ostringstream* oss = buffer.get();
  (*oss) << data;
  return *this;
}

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
Logger& Logger::Print(bool newLine)
{
  std::string timestamp = Timestamp();
  
  if (!buffer.get()) buffer.reset(new std::ostringstream);
  std::ostringstream* oss = buffer.get();
  
  {
    boost::lock_guard<boost::mutex> lock(outMutex);
    if (out == &std::clog) out = 0;
    if (!out)
    {
      assert(!path.empty() && "Log path must be set with Logger::SetPath()");
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
    }
    (*out) << (newLine ? "\n" : "") << std::flush;
  }
  
  oss->str("");
  return *this;
}


Logger& flush(Logger& logger)
{
	return logger.Print(false);
}

Logger& endl(Logger& logger)
{
  return logger.Print(true);
}

void Initialise(const std::string& dataPath)
{
  ftpd.SetPath(dataPath + "/access.log");
  access.SetPath(dataPath + "/access.log");
  siteop.SetPath(dataPath + "/siteop.log");
  error.SetPath(dataPath + "/error.log");
}

}

#ifdef LOGGER_TEST

#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

void Test()
{
  for (int i = 0; i < 1000; ++i)
  {
    logger::ftpd << i << logger::endl;
//    boost::this_thread::sleep(boost::posix_time::seconds(1));
  }
  
  logger::ftpd << logger::flush;
}

int main()
{
  logger::Initialise("/home/bioboy/ftpd/tmp/logs");
  logger::ftpd << "test" << logger::endl;
  logger::ftpd << std::fixed << std::setprecision(0) << 69.59 << logger::endl;
  
  boost::thread t1(&Test);
  boost::thread t2(&Test);
  
  t1.join();
  t2.join();
}

#endif
