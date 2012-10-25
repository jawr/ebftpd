#ifndef __DB_STATS_STAT_HPP
#define __DB_STATS_STAT_HPP

#include "acl/user.hpp"
#include "stats/stat.hpp"

namespace db { namespace stats
{
  void Upload(const acl::User& user, long long kbytes, double xfertime);
  void Download(const acl::User& user, long long kbytes, double xfertime);

  // we need to pass the creation date of the file to this in order to effect
  // the correct stats segment.
  void UploadDecr(const acl::User& user, long long kbytes);

// end
}
}
#endif
