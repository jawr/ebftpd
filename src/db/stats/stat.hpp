#ifndef __DB_STATS_STAT_HPP
#define __DB_STATS_STAT_HPP

#include <map>
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

  std::map<acl::UserID, ::stats::Stat> GetAllDown(const std::vector<acl::User>& users);
  std::map<acl::UserID, ::stats::Stat> GetAllUp(const std::vector<acl::User>& users);

  void Upload(const acl::User& user, long long kbytes, long long xfertime);
  void Download(const acl::User& user, long long kbytes, long long xfertime);

  // we need to pass the creation date of the file to this in order to effect
  // the correct stats segment.
  void UploadDecr(const acl::User& user, long long kbytes);

// end
}
}
#endif
