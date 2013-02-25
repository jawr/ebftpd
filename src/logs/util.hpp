#ifndef __LOGS_UTIL_HPP
#define __LOGS_UTIL_HPP

#include <string>
#include <ctime>

namespace logs
{

inline std::string Timestamp()
{
  time_t now = time(nullptr);
  char buf[26];
  strftime(buf,sizeof(buf),"%a %b %d %T %Y", localtime(&now));
  return buf;
}

} /* logs namespace */

#endif
