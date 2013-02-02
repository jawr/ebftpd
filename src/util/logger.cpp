#include <cassert>
#include <iomanip>
#include <fstream>
#include <ctime>
#include "util/logger.hpp"

namespace util { namespace logger
{

Logger::Logger() : out(&std::cout), stdoutAlso(false) { }
Logger::Logger(const std::string& path, bool stdoutAlso) : 
  path(path), out(&std::cout), stdoutAlso(stdoutAlso) { }


void Logger::SetPath(const std::string& path, bool stdoutAlso)
{
  this->path = path;
  this->stdoutAlso = stdoutAlso;
}

Logger::~Logger()
{
  if (out != &std::cout) delete out;
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
  time_t now = time(nullptr);
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
    if (out == &std::cout && !path.empty()) out = nullptr;
    if (!out)
    {
      std::ofstream* fout(new std::ofstream(path.c_str(), std::ios::app));
      if (*fout) out = fout;
      else
      {
        delete fout;
        fout = nullptr;
        out = &std::cout;
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
      if (stdoutAlso && out != &std::cout)
        std::cout << timestamp << " " << line;
    }

    (*out) << (newLine ? "\n" : "") << std::flush;
    if (stdoutAlso && out != &std::cout)
      std::cout << (newLine ? "\n" : "") << std::flush;
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
