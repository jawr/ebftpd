//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
#include "db/user/util.hpp"
#include "acl/flags.hpp"

namespace acl { namespace message
{

fs::Path Evaluate(const std::vector<cfg::Right>& rights, const User& user)
{
  auto info = user.ACLInfo();
  for (const auto& right : rights)
  {
    if (right.ACL().Evaluate(info)) return fs::Path(right.Path());
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

int Max(const std::vector<cfg::ACLInt>& maxStats, const User& user)
{
  auto info = user.ACLInfo();
  for (const auto& maxStat : maxStats)
  {
    if (maxStat.ACL().Evaluate(info)) return maxStat.Arg();
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

std::vector<const cfg::SpeedLimit*>
UploadMaximum(const User& user, const fs::Path& path)
{
  auto info = user.ACLInfo();
  std::vector<const cfg::SpeedLimit*> matches;
  if (!user.HasFlag(acl::Flag::Exempt))
  {
    for (const auto& limit : cfg::Get().MaximumSpeed())
    {
      if (limit.Uploads() > 0 && limit.ACL().Evaluate(info) &&
          util::WildcardMatch(limit.Path(), path.ToString()))
      {
        matches.emplace_back(&limit);
      }
    }
  }
  return matches;
}

std::vector<const cfg::SpeedLimit*>
DownloadMaximum(const User& user, const fs::Path& path)
{
  auto info = user.ACLInfo();
  std::vector<const cfg::SpeedLimit*> matches;
  if (!user.HasFlag(acl::Flag::Exempt))
  {
    for (const auto& limit : cfg::Get().MaximumSpeed())
    {
      if (limit.Downloads() > 0 && limit.ACL().Evaluate(info) &&
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
  auto info = user.ACLInfo();
  for (const auto& limit : cfg::Get().MinimumSpeed())
  {
    if (limit.ACL().Evaluate(info) &&
        util::WildcardMatch(limit.Path(), path.ToString()))
    {
      return limit.Uploads();
    }
  }
  return 0;
}

int DownloadMinimum(const User& user, const fs::Path& path)
{
  auto info = user.ACLInfo();
  for (const auto& limit : cfg::Get().MinimumSpeed())
  {
    if (limit.ACL().Evaluate(info) &&
        util::WildcardMatch(limit.Path(), path.ToString()))
    {
      return limit.Downloads();
    }
  }
  return 0;
}

} /* speed namespace */

bool AllowFxp(const User& user, bool& logging, 
  const std::function<bool(const cfg::AllowFxp&)>& isAllowed)
{
  auto info = user.ACLInfo();
  const cfg::Config& config = cfg::Get();
  for (const auto& af : config.AllowFxp())
  {
    if (af.ACL().Evaluate(info))
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
      [](const cfg::AllowFxp& af) { return af.Uploads(); });
}

bool AllowFxpReceive(const User& user, bool& logging)
{
  return AllowFxp(user, logging, 
      [](const cfg::AllowFxp& af) { return af.Downloads(); });
}

bool AllowSiteCmd(const User& user, const std::string& keyword)
{
  auto info = user.ACLInfo();
  std::vector<std::string> toks;
  util::Split(toks, keyword, "|");
  if (toks.empty()) return true;
  for (auto& tok : toks)
  {
    try
    {
      if (cfg::Get().CommandACL(tok).Evaluate(info)) return true;
    }
    catch (const std::out_of_range&) { }
  }
  return false;
}

boost::optional<const cfg::Creditcheck&> 
CreditCheck(const User& user, const fs::VirtualPath& path)
{
  auto info = user.ACLInfo();
  for (const auto& cc : cfg::Get().Creditcheck())
  {
    if (util::WildcardMatch(cc.Path(), path.ToString()) &&
        cc.ACL().Evaluate(info))
    {
      return boost::optional<const cfg::Creditcheck&>(cc);
    }
  }
  return boost::optional<const cfg::Creditcheck&>();
}

boost::optional<const cfg::Creditloss&> 
CreditLoss(const User& user, const fs::VirtualPath& path)
{
  auto info = user.ACLInfo();
  for (const auto& cc : cfg::Get().Creditloss())
  {
    if (util::WildcardMatch(cc.Path(), path.ToString()) &&
        cc.ACL().Evaluate(info))
    {
      return boost::optional<const cfg::Creditloss&>(cc);
    }
  }
  return boost::optional<const cfg::Creditloss&>();
}

bool SecureIP(const User& user, const std::string& ip, IPStrength& minimum)
{
  auto info = user.ACLInfo();
  IPStrength strength(ip);
  for (auto& si : cfg::Get().SecureIp())
    if (si.ACL().Evaluate(info))
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
  auto info = user.ACLInfo();
  PasswdStrength strength(password);
  for (auto& sp : cfg::Get().SecurePass())
    if (sp.ACL().Evaluate(info))
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
