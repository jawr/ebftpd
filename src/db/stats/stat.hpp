#ifndef __DB_STATS_STAT_HPP
#define __DB_STATS_STAT_HPP

#include <unordered_map>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include "acl/user.hpp"
#include "db/types.hpp"
#include "stats/stat.hpp"

namespace db { namespace stats
{
  void Get(mongo::Query& query, QueryResults& results);
  
  mongo::BSONObj GetFromCommand(const mongo::BSONObj& match);

  ::stats::Stat GetWeekDown(acl::UserID uid, int week, int year);
  ::stats::Stat GetWeekUp(acl::UserID uid, int week, int year);

  ::stats::Stat GetAllDown(const acl::User& user);
  ::stats::Stat GetAllUp(const acl::User& user);

  std::unordered_map<acl::UserID, ::stats::Stat> GetAllDown(const std::vector<acl::User>& users);
  std::unordered_map<acl::UserID, ::stats::Stat> GetAllUp(const std::vector<acl::User>& users);

  void Upload(const acl::User& user, long long bytes, long long xfertime, const std::string& section);
  void Download(const acl::User& user, long long bytes, long long xfertime, const std::string& section);

  // we need to pass the creation date of the file to this in order to effect
  // the correct stats segment.
  void UploadDecr(const acl::User& user, long long bytes);

// end
}
}
#endif
