#ifndef __ACL_MISC_HPP
#define __ACL_MISC_HPP

#include "util/error.hpp"
#include "ftp/client.hpp"

namespace cfg { namespace setting
{
class SpeedLimit;
}
}

namespace acl { namespace message
{

enum Type
{
  Welcome,
  Goodbye
};

template <Type type>
fs::Path Choose(const User& user);

} /* message namespace */

namespace stats
{

int MaxUsers(const User& user);
int MaxGroups(const User& user);

} /* stats namespace */

namespace speed
{

std::vector<const cfg::setting::SpeedLimit*>
UploadLimit(const User& user, const fs::Path& path);

std::vector<const cfg::setting::SpeedLimit*>
DownloadLimit(const User& user, const fs::Path& path);

} /* speed namespace */
} /* acl namespace */

#endif
