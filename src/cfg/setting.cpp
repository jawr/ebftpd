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

#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
#include "cfg/setting.hpp"
#include "cfg/error.hpp"
#include "util/string.hpp"
#include "cfg/util.hpp"
#include "cfg/defaults.hpp"
#include "util/misc.hpp"

namespace cfg
{

Database::Database(const char* name, const char* address, int port, const char* login, const char* password) :
  name(name), 
  login(login),
  password(password)
{
  hosts.emplace_back(address, port);
}

std::string Database::URL() const
{
  assert(!hosts.empty() && (hosts.size() < 2 || !replicaSet.empty()));
  std::ostringstream os;
  if (!replicaSet.empty()) os << replicaSet << '/';
  bool firstHost = true;
  for (const auto& host : hosts)
  {
    if (!firstHost) os << ',';
    os << host.first << ':' << host.second;
    firstHost = false;
  }
  return os.str();
}

bool Database::NeedAuth() const
{
  if (login.empty()) return false;
  assert(!password.empty());
  return true;
}

bool Database::operator==(const Database& rhs) const
{
  return name == rhs.name &&
         hosts == rhs.hosts &&
         login == rhs.login &&
         password == rhs.password &&
         replicaSet == rhs.replicaSet;
}

AsciiDownloads::AsciiDownloads(const std::vector<std::string>& toks) :
  kBytes(-1)
{
  try
  {
    kBytes = ParseSize(toks[0]);
  }
  catch (const std::bad_cast&) { }
  if (kBytes == 0) kBytes = -1;
  masks.assign(toks.begin() + 1, toks.end());
}

bool AsciiDownloads::Allowed(off_t size, const std::string& path) const
{
  if (kBytes > 0 && size / 1024 > kBytes) return false;
  if (masks.empty()) return true;
  for (auto& mask : masks)
  {
    if (util::WildcardMatch(mask, path)) return true;
  }
  return false;
}


AsciiUploads::AsciiUploads(const std::vector<std::string>& toks) :
  masks(toks)
{
}

bool AsciiUploads::Allowed(const std::string& path) const
{
  if (masks.empty()) return true;
  for (auto& mask : masks)
  {
    if (util::WildcardMatch(mask, path)) return true;
  }
  return false;
}

SecureIp::SecureIp(std::vector<std::string> toks)
{
  int numOctets = util::StrToInt(toks[0]);
  if (numOctets < 0) throw std::bad_cast();
  bool isHostname = YesNoToBoolean(toks[1]);
  bool hasIdent = YesNoToBoolean(toks[2]);
  strength = acl::IPStrength(numOctets, isHostname, hasIdent);
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL(util::Join(toks, " "));
}

SecurePass::SecurePass(std::vector<std::string> toks) :
  strength(toks[0])
{
  toks.erase(toks.begin());
  acl = acl::ACL(util::Join(toks, " "));
}

SpeedLimit::SpeedLimit(std::vector<std::string> toks) :
  path(toks[0]),
  downloads(ParseSize(toks[1])),
  uploads(ParseSize(toks[2]))
{
  toks.erase(toks.begin(), toks.begin() + 3);
  acl = acl::ACL(util::Join(toks, " "));
}

SimXfers::SimXfers(int maxDownloads, int maxUploads) : 
  maxDownloads(maxDownloads),
  maxUploads(maxUploads) 
{
}

SimXfers::SimXfers(std::vector<std::string> toks)
{
  maxDownloads = util::StrToInt(toks[0]);
  maxUploads = util::StrToInt(toks[1]);
  if (maxDownloads < -1 || maxUploads < -1) throw std::bad_cast();
}

PasvAddr::PasvAddr(const std::vector<std::string>& toks) :
  addr(toks[0])
{
}

Ports::Ports(const std::vector<std::string>& toks)   
{
  std::vector<std::string> temp;
  for (const auto& token : toks)
  {
    temp.clear();
    util::Split(temp, token, "-");
    if (temp.size() > 2) throw cfg::ConfigError("Invalid port range.");
    int from = util::StrToInt(temp[0]);
    int to = from;
    if (temp.size() > 1) to = util::StrToInt(temp[1]);
    if (to < from)
      throw cfg::ConfigError("To port lower than from port in port range.");
    if (to < 1024 || from < 1024 || to > 65535 || from > 65535)
      throw std::bad_cast();
    ranges.emplace_back(from, to);
  }
}

AllowFxp::AllowFxp(bool downloads, bool uploads, bool logging, const char* acl) :
  downloads(downloads), 
  uploads(uploads), 
  logging(logging),
  acl(acl)
{
}

AllowFxp::AllowFxp(std::vector<std::string> toks)   
{
  downloads = YesNoToBoolean(toks[0]);
  uploads   = YesNoToBoolean(toks[1]);
  logging   = YesNoToBoolean(toks[2]);
  toks.erase(toks.begin(), toks.begin() + 3);
  acl = acl::ACL(util::Join(toks, " "));
}

Alias::Alias(const std::vector<std::string>& toks)   
{
  name = util::ToLowerCopy(toks[0]);
  path = toks[1];
}

Right::Right(std::vector<std::string> toks)
{
  path = toks[0];
  toks.erase(toks.begin());
  acl = acl::ACL(util::Join(toks, " "));
  specialVar = path.find("[:username:]") != std::string::npos ||
               path.find("[:groupname:]") != std::string::npos;
}

PathFilter::PathFilter(const char* regex, const char* acl) :
  regex(new boost::regex(regex)),
  acl(acl)
{
}

PathFilter::~PathFilter()
{
}

PathFilter& PathFilter::operator=(const PathFilter& rhs)
{
  regex.reset(new boost::regex(*rhs.regex));
  acl = rhs.acl;
  return *this;
}

PathFilter& PathFilter::operator=(PathFilter&& rhs)
{
  regex = std::move(rhs.regex);
  acl = std::move(rhs.acl);
  return *this;
}

PathFilter::PathFilter(const PathFilter& other) :
  regex(new boost::regex(*other.regex)),
  acl(other.acl)
{
}

PathFilter::PathFilter(PathFilter&& other) :
  regex(std::move(other.regex)),
  acl(std::move(other.acl))
{
}

PathFilter::PathFilter(std::vector<std::string> toks)   
{
  try
  {
    regex.reset(new boost::regex(toks[0]));
  }
  catch (const boost::regex_error&)
  {
    throw ConfigError("Invalid regular expression.");
  }
  toks.erase(toks.begin(), toks.begin() + 1);
  acl = acl::ACL(util::Join(toks, " "));
}

const boost::regex& PathFilter::Regex() const { return *regex; }

MaxUsers::MaxUsers(int users, int exemptUsers) : 
  users(users), 
  exemptUsers(exemptUsers)
{
}

MaxUsers::MaxUsers(const std::vector<std::string>& toks)   
{
  users = util::StrToInt(toks[0]);
  if (users < 0) throw std::bad_cast();
  exemptUsers = util::StrToInt(toks[1]);
  if (exemptUsers < 0) throw std::bad_cast();
}

ACLInt::ACLInt(std::vector<std::string> toks)   
{
  arg = util::StrToInt(toks[0]);
  toks.erase(toks.begin());
  acl = acl::ACL(util::Join(toks, " ")); 
}

Lslong::Lslong(const char* options, int maxRecursion) : 
  options(options),
  maxRecursion(maxRecursion)
{
}

Lslong::Lslong(std::vector<std::string> toks)   
{
  options = toks[0];
  if (options[0] == '-') options.erase(0, 1);
  if (toks.size() == 1) return;
  
  maxRecursion = util::StrToInt(toks[1]);
  if (maxRecursion < 0) throw std::bad_cast();
}

HiddenFiles::HiddenFiles(std::vector<std::string> toks)   
{
  path = toks[0];
  toks.erase(toks.begin());
  masks = toks;
}

Creditcheck::Creditcheck(std::vector<std::string> toks)   
{
  path = toks[0];
  ratio = util::StrToInt(toks[1]);
  if (ratio < 0) throw std::bad_cast();
  toks.erase(toks.begin(), toks.begin()+2);
  acl = acl::ACL(util::Join(toks, " "));
}

Creditloss::Creditloss(std::vector<std::string> toks)   
{
  path = toks[0];
  ratio = util::StrToInt(toks[1]);
  if (ratio < 0) throw std::bad_cast();
  toks.erase(toks.begin(), toks.begin()+2);
  acl = acl::ACL(util::Join(toks, " "));
}

NukeStyle::NukeStyle(const std::string& format, Action action, 
                     long long emptyKBytes, long long emptyPenalty) :
  format(format),
  action(action), 
  emptyKBytes(emptyKBytes),
  emptyPenalty(emptyPenalty)
{
}

NukeStyle::NukeStyle(const std::vector<std::string>& toks)   
{
  format = toks[0];
  std::string action = util::ToLowerCopy(toks[1]);
  if (action == "deleteall") action = DeleteAll;
  else if (action == "keepdir") action = KeepDir;
  else if (action == "keep") action = Keep;
  else throw std::bad_cast();
  emptyKBytes = ParseSize(toks[2]);
  emptyPenalty = ParseSize(toks[3]);
}

Msgpath::Msgpath(const std::vector<std::string>& toks)   
{
  path = toks[0];
  filepath = toks[1];
  acl = acl::ACL(util::Join(toks, " "));
}

Privpath::Privpath(std::vector<std::string> toks)   
{
  path = toks[0];
  toks.erase(toks.begin());
  acl = acl::ACL(util::Join(toks, " "));
}

SiteCmd::SiteCmd(const std::vector<std::string>& toks)   
{
  std::vector<std::string> args;
  util::Split(args, toks[0], " ", true);
  util::ToUpper(args[0]);

  command = args[0];
  syntax = util::Join(args, " ");
  
  description = toks[1];
  std::string typeStr(util::ToUpperCopy(toks[2]));
  if (typeStr == "EXEC") type = Type::Exec;
  else if (typeStr == "TEXT") type = Type::Text;
  else if (typeStr == "ALIAS")
  {
    type = Type::Alias;
    util::ToUpper(target);
  }
  else
    throw std::bad_cast();
  target = toks[3];
  if (toks.size() == 5) arguments = toks[4];
}

Cscript::Cscript(const std::vector<std::string>& toks)   
{
  command = util::ToUpperCopy(toks[0]);
  std::string when = util::ToLowerCopy(toks[1]);
  if (when == "pre") type = Type::Pre;
  else if (when == "post") type = Type::Post;
  else throw std::bad_cast();
  path = toks[2];
}

struct IdleTimeoutImpl
{
  boost::posix_time::seconds maximum;
  boost::posix_time::seconds minimum;
  boost::posix_time::seconds timeout;

  IdleTimeoutImpl(long maximum, long minimum, long timeout) :
    maximum(maximum), 
    minimum(minimum),
    timeout(timeout)
  { }

  IdleTimeoutImpl(const std::vector<std::string>& toks) :
    maximum(boost::posix_time::seconds(util::StrToLong(toks[0]))),
    minimum(boost::posix_time::seconds(util::StrToLong(toks[1]))),
    timeout(boost::posix_time::seconds(util::StrToLong(toks[2])))
  {
    if (timeout.total_seconds() < 1 || minimum.total_seconds() < 1 || maximum.total_seconds() < 1)
      throw std::bad_cast();
    if (minimum >= maximum)
      throw ConfigError("Mnimum must be smaller than maximum in idle_timeout");
    if (timeout < minimum || timeout > maximum)
      throw ConfigError("Default timeout must be larger than or equal to minimum "
                        "and smaller than or equal to maximum in idle_timeout");
  }
};

IdleTimeout::IdleTimeout(long maximum, long minimum, long timeout) :
  pimpl(new IdleTimeoutImpl(maximum, minimum, timeout))
{
}

IdleTimeout::IdleTimeout(const std::vector<std::string>& toks) :
  pimpl(new IdleTimeoutImpl(toks))
{
}

IdleTimeout::~IdleTimeout()
{
}

IdleTimeout& IdleTimeout::operator=(const IdleTimeout& rhs)
{
  pimpl.reset(new IdleTimeoutImpl(*rhs.pimpl));
  return *this;
}

IdleTimeout& IdleTimeout::operator=(IdleTimeout&& rhs)
{
  pimpl = std::move(rhs.pimpl);
  return *this;
}

IdleTimeout::IdleTimeout(const IdleTimeout& other) :
  pimpl(new IdleTimeoutImpl(*other.pimpl))
{
}

IdleTimeout::IdleTimeout(IdleTimeout&& other) :
  pimpl(std::move(other.pimpl))
{
}

boost::posix_time::seconds IdleTimeout::Maximum() const { return pimpl->maximum; }
boost::posix_time::seconds IdleTimeout::Minimum() const { return pimpl->minimum; }
boost::posix_time::seconds IdleTimeout::Timeout() const { return pimpl->timeout; }

CheckScript::CheckScript(const std::vector<std::string>& toks) :
  path(toks[0]), 
  mask(toks.size() == 2 ? toks[1] : "*"), 
  disabled(toks[0] == "none")
{
}

Log::Log(const std::string& name, const std::vector<std::string>& toks) :
  name(name),
  console(YesNoToBoolean(toks[0])),
  file(YesNoToBoolean(toks[1])),
  database(toks.size() >= 3 && util::StrToLong(toks[2]))
{
  if (database < 0) throw std::bad_cast();
}

TransferLog::TransferLog(const std::string& name, const std::vector<std::string>& toks) :
  Log(name, toks),
  uploads(YesNoToBoolean(toks[3])),
  downloads(YesNoToBoolean(toks[4]))
{
}

NukeMax::NukeMax(const std::vector<std::string>& toks) :
  multiplier(util::StrToInt(toks[0])),
  percent(util::StrToInt(toks[1]))
{
  if (multiplier < 0 || percent < 0 || percent > 100) throw std::bad_cast();
}


bool NukeMax::IsOkay(int value, bool isPercent) const
{
  if (isPercent) return value >= 0 && value <= percent;
  else return value >= 0 && value <= multiplier;
}
}