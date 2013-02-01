#ifndef __DB_DUPE_DUPE_HPP
#define __DB_DUPE_DUPE_HPP

#include <string>

namespace db { namespace dupe
{

void Add(const std::string& directory, const std::string& section);

struct DupeResult
{
  std::string directory;
  std::string section;
  boost::posix_time::ptime dateTime;
  
  DupeResult(const std::string& directory, const std::string& section, 
               const boost::posix_time::ptime& dateTime) :
    directory(directory), section(section), dateTime(dateTime)
  { }
};

std::vector<DupeResult> Search(const std::vector<std::string>& terms, int limit);

} /* dupe namespace */
} /* db namespace */

#endif
