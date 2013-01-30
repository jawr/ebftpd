#include <cassert>
#include <boost/algorithm/string/predicate.hpp>
#include "acl/misc.hpp"
#include "acl/user.hpp"
#include "fs/path.hpp"
#include "fs/owner.hpp"
#include "cfg/get.hpp"
#include "util/string.hpp"

namespace acl { namespace message
{

fs::Path Evaluate(const std::vector<cfg::setting::Right>& rights, const User& user)
{
  for (const auto& right : rights)
  {
    if (right.ACL().Evaluate(user)) return right.Path();
  }
  return fs::Path();
}

template <Type type>
struct Traits;

template <>
struct Traits<Welcome>
{
  static fs::Path Choose(const User& user)
  {
    return Evaluate(cfg::Get().WelcomeMsg(), user);
  }
};

template <>
struct Traits<Goodbye>
{
  static fs::Path Choose(const User& user)
  {
    return Evaluate(cfg::Get().GoodbyeMsg(), user);
  }
};

template <Type type>
fs::Path Choose(const User& user)
{
  return Traits<type>::Choose(user);
}

template fs::Path Choose<Welcome>(const User& user);
template fs::Path Choose<Goodbye>(const User& user);

} /* message namespace */

namespace stats
{

int Max(const std::vector<cfg::setting::ACLInt>& maxStats, const User& user)
{
  for (const auto& maxStat : maxStats)
  {
    if (maxStat.ACL().Evaluate(user)) return maxStat.Arg();
  }
  return -1;
}

int MaxUsers(const User& user)
{
  return Max(cfg::Get().MaxUstats(), user);
}

int MaxGroups(const User& user)
{
  return Max(cfg::Get().MaxGstats(), user);
}

} /* stats namespace */

namespace speed
{

std::vector<const cfg::setting::SpeedLimit*>
UploadLimit(const User& user, const fs::Path& path)
{
  std::vector<const cfg::setting::SpeedLimit*> matches;
  if (!user.CheckFlag(acl::Flag::Exempt))
  {
    for (const auto& limit : cfg::Get().SpeedLimit())
    {
      if (limit.UlLimit() > 0 && limit.ACL().Evaluate(user) &&
          util::string::WildcardMatch(limit.Path().ToString(), path.ToString()))
      {
        matches.emplace_back(&limit);
      }
    }
  }
  return matches;
}

std::vector<const cfg::setting::SpeedLimit*>
DownloadLimit(const User& user, const fs::Path& path)
{
  std::vector<const cfg::setting::SpeedLimit*> matches;
  if (!user.CheckFlag(acl::Flag::Exempt))
  {
    for (const auto& limit : cfg::Get().SpeedLimit())
    {
      if (limit.DlLimit() > 0 && limit.ACL().Evaluate(user) &&
          util::string::WildcardMatch(limit.Path().ToString(), path.ToString()))
      {
        matches.emplace_back(&limit);
      }
    }
  }
  return matches;
}

} /* speed namespace */
} /* acl namespace */
