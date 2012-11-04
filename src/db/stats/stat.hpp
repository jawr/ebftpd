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

  ::stats::Stat GetWeekDown(acl::UserID uid, int week, int year);
  ::stats::Stat GetWeekUp(acl::UserID uid, int week, int year);

  void GetAllDown(const boost::ptr_vector<acl::User>& users,
    std::map<acl::UserID, ::stats::Stat>& stats);
  void GetAllUp(const boost::ptr_vector<acl::User>& users,
    std::map<acl::UserID, ::stats::Stat>& stats);

  void Upload(const acl::User& user, long long kbytes, long long xfertime);
  void Download(const acl::User& user, long long kbytes, long long xfertime);

  // we need to pass the creation date of the file to this in order to effect
  // the correct stats segment.
  void UploadDecr(const acl::User& user, long long kbytes);

// end
}
}
#endif
