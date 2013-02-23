#include <cassert>
#include <functional>
#include <vector>
#include <boost/optional.hpp>
#include "acl/misc.hpp"
#include "util/string.hpp"
#include "acl/user.hpp"
#include "fs/path.hpp"
#include "fs/owner.hpp"
#include "cfg/get.hpp"
#include "acl/ipstrength.hpp"
#include "acl/passwdstrength.hpp"
#include "db/userutil.hpp"
#include "acl/flags.hpp"

namespace acl { namespace message
{

fs::Path Evaluate(const std::vector<cfg::setting::Right>& rights, const User& user)
{
  for (const auto& right : rights)
  {
    if (right.ACL().Evaluate(user)) return fs::Path(right.Path());
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
UploadMaximum(const User& user, const fs::Path& path)
{
  std::vector<const cfg::setting::SpeedLimit*> matches;
  if (!user.HasFlag(acl::Flag::Exempt))
  {
    for (const auto& limit : cfg::Get().MaximumSpeed())
    {
      if (limit.UlLimit() > 0 && limit.ACL().Evaluate(user) &&
          util::WildcardMatch(limit.Path(), path.ToString()))
      {
        matches.emplace_back(&limit);
      }
    }
  }
  return matches;
}

std::vector<const cfg::setting::SpeedLimit*>
DownloadMaximum(const User& user, const fs::Path& path)
{
  std::vector<const cfg::setting::SpeedLimit*> matches;
  if (!user.HasFlag(acl::Flag::Exempt))
  {
    for (const auto& limit : cfg::Get().MaximumSpeed())
    {
      if (limit.DlLimit() > 0 && limit.ACL().Evaluate(user) &&
          util::WildcardMatch(limit.Path(), path.ToString()))
      {
        matches.emplace_back(&limit);
      }
    }
  }
  return matches;
}

int UploadMinimum(const User& user, const fs::Path& path)
{
  for (const auto& limit : cfg::Get().MinimumSpeed())
  {
    if (limit.ACL().Evaluate(user) &&
        util::WildcardMatch(limit.Path(), path.ToString()))
    {
      return limit.UlLimit();
    }
  }
  return 0;
}

int DownloadMinimum(const User& user, const fs::Path& path)
{
  for (const auto& limit : cfg::Get().MinimumSpeed())
  {
    if (limit.ACL().Evaluate(user) &&
        util::WildcardMatch(limit.Path(), path.ToString()))
    {
      return limit.DlLimit();
    }
  }
  return 0;
}

} /* speed namespace */

bool AllowFxp(const User& user, bool& logging, 
  const std::function<bool(const cfg::setting::AllowFxp&)>& isAllowed)
{
  const cfg::Config& config = cfg::Get();
  for (const auto& af : config.AllowFxp())
  {
    if (af.ACL().Evaluate(user))
    {
      logging = af.Logging();
      return isAllowed(af);
    }
  }
  
  return false;
}

bool AllowFxpSend(const User& user, bool& logging)
{
  return AllowFxp(user, logging, 
      [](const cfg::setting::AllowFxp& af) { return af.Uploads(); });
}

bool AllowFxpReceive(const User& user, bool& logging)
{
  return AllowFxp(user, logging, 
      [](const cfg::setting::AllowFxp& af) { return af.Downloads(); });
}

bool AllowSiteCmd(const User& user, const std::string& keyword)
{
  std::vector<std::string> toks;
  util::Split(toks, keyword, "|");
	if (toks.empty()) return true;
  for (auto& tok : toks)
  {
    try
    {
      if (cfg::Get().CommandACL(tok).Evaluate(user)) return true;
    }
    catch (const std::out_of_range&) { }
  }
  return false;
}

boost::optional<const cfg::setting::Creditcheck&> 
CreditCheck(const User& user, const fs::VirtualPath& path)
{
  for (const auto& cc : cfg::Get().Creditcheck())
  {
    if (util::WildcardMatch(cc.Path(), path.ToString()) &&
        cc.ACL().Evaluate(user))
    {
      return boost::optional<const cfg::setting::Creditcheck&>(cc);
    }
  }
  return boost::optional<const cfg::setting::Creditcheck&>();
}

boost::optional<const cfg::setting::Creditloss&> 
CreditLoss(const User& user, const fs::VirtualPath& path)
{
  for (const auto& cc : cfg::Get().Creditloss())
  {
    if (util::WildcardMatch(cc.Path(), path.ToString()) &&
        cc.ACL().Evaluate(user))
    {
      return boost::optional<const cfg::setting::Creditloss&>(cc);
    }
  }
  return boost::optional<const cfg::setting::Creditloss&>();
}

bool SecureIP(const User& user, const std::string& ip, IPStrength& minimum)
{
  IPStrength strength(ip);
  for (auto& si : cfg::Get().SecureIp())
    if (si.ACL().Evaluate(user))
    {
      if (si.Strength().Allowed(strength))
        return true;
      else
      {
        minimum = si.Strength();
        return false;
      }
    }
  return true;
}

bool SecurePass(const User& user, const std::string& password , PasswdStrength& minimum)
{
  PasswdStrength strength(password);
  for (auto& sp : cfg::Get().SecurePass())
    if (sp.ACL().Evaluate(user))
    {
      if (sp.Strength().Allowed(strength))
        return true;
      else
      {
        minimum = sp.Strength();
        return false;
      }
    }
  return true;
}

bool IPAllowed(const std::string& address)
{
  return db::IdentIPAllowed("*@" + address);
}

bool IdentIPAllowed(acl::UserID uid, const std::string& identAddress)
{
  return db::IdentIPAllowed(identAddress, uid);
}

} /* acl namespace */
