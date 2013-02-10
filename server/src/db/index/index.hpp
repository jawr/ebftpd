#ifndef __DB_INDEX_INDEX_HPP
#define __DB_INDEX_INDEX_HPP

#include <string>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace db { namespace index
{

void Add(const std::string& path, const std::string& section);
void Delete(const std::string& path);

struct SearchResult
{
  std::string path;
  std::string section;
  boost::posix_time::ptime dateTime;
  
  SearchResult(const std::string& path, const std::string& section, 
               const boost::posix_time::ptime& dateTime) :
    path(path), section(section), dateTime(dateTime)
  { }
};

std::vector<SearchResult> Search(const std::vector<std::string>& terms, int limit);
std::vector<SearchResult> Newest(int limit);

} /* index namespace */
} /* db namespace */

#endif
