#ifndef __ACL_MISC_HPP
#define __ACL_MISC_HPP

#include <string>
#include <boost/optional.hpp>

namespace cfg { namespace setting
{
class SpeedLimit;
}
}

namespace fs
{
class VirtualPath;
}

namespace acl
{

class User;

namespace message
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
UploadMaximum(const User& user, const fs::Path& path);

std::vector<const cfg::setting::SpeedLimit*>
DownloadMaximum(const User& user, const fs::Path& path);

int UploadMinimum(const User& user, const fs::Path& path);

int DownloadMinimum(const User& user, const fs::Path& path);

} /* speed namespace */


bool AllowFxpSend(const User& user, bool& logging);
bool AllowFxpReceive(const User& user, bool& logging);
bool AllowSiteCmd(const User& user, const std::string& keyword);

boost::optional<const cfg::setting::Creditcheck&> 
CreditCheck(const User& user, const fs::VirtualPath& path);

boost::optional<const cfg::setting::Creditloss&> 
CreditLoss(const User& user, const fs::VirtualPath& path);

class IPStrength;
bool SecureIP(const User& user, const std::string& ip, IPStrength& minimum);

class PasswdStrength;

bool SecurePass(const User& user, const std::string& password, PasswdStrength& minimum);

} /* acl namespace */

#endif
