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

#include <sstream>
#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
#include "cfg/setting.hpp"
#include "cfg/error.hpp"
#include "util/string.hpp"
#include "cfg/util.hpp"
#include "cfg/defaults.hpp"

namespace cfg
{

Database::Database(const char* name, const char* address, int port, const char* login, const char* password) :
  name(name), 
  address(address), 
  port(port),
  login(login),
  password(password)
{
  std::ostringstream os;
  os << address << ":" << port;
  host = os.str();
}

Database::Database(const std::vector<std::string>& toks) : port(-1)
{
  name = toks[0];
  address = toks[1];

  port = boost::lexical_cast<int>(toks[2]);
  if (port < 0 || port >= 65535) throw boost::bad_lexical_cast();

  std::ostringstream os;
  os << address << ":" << port;
  host = os.str();
  
  if (toks.size() == 3) return;
  if (toks.size() != 5) throw ConfigError("Wrong numer of Parameters for database");
  
  login = toks[3];
  password = toks[4];
}

bool Database::NeedAuth() const
{
  if (login.empty()) return false;
  assert(!password.empty());
  return true;
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
  int numOctets = boost::lexical_cast<int>(toks[0]);
  if (numOctets < 0) throw boost::bad_lexical_cast();
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
  maxDownloads = boost::lexical_cast<int>(toks[0]);
  maxUploads = boost::lexical_cast<int>(toks[1]);
  if (maxDownloads < -1 || maxUploads < -1) throw boost::bad_lexical_cast();
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
    int from = boost::lexical_cast<int>(temp[0]);
    int to = from;
    if (temp.size() > 1) to = boost::lexical_cast<int>(temp[1]);
    if (to < from)
      throw cfg::ConfigError("To port lower than from port in port range.");
    if (to < 1024 || from < 1024 || to > 65535 || from > 65535)
      throw boost::bad_lexical_cast();
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
  users = boost::lexical_cast<int>(toks[0]);
  if (users < 0) throw boost::bad_lexical_cast();
  exemptUsers = boost::lexical_cast<int>(toks[1]);
  if (exemptUsers < 0) throw boost::bad_lexical_cast();
}

ACLInt::ACLInt(std::vector<std::string> toks)   
{
  arg = boost::lexical_cast<int>(toks[0]);
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
  
  maxRecursion = boost::lexical_cast<int>(toks[1]);
  if (maxRecursion < 0) throw boost::bad_lexical_cast();
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
  ratio = boost::lexical_cast<int>(toks[1]);
  if (ratio < 0) throw boost::bad_lexical_cast();
  toks.erase(toks.begin(), toks.begin()+2);
  acl = acl::ACL(util::Join(toks, " "));
}

Creditloss::Creditloss(std::vector<std::string> toks)   
{
  path = toks[0];
  ratio = boost::lexical_cast<int>(toks[1]);
  if (ratio < 0) throw boost::bad_lexical_cast();
  toks.erase(toks.begin(), toks.begin()+2);
  acl = acl::ACL(util::Join(toks, " "));
}

NukedirStyle::NukedirStyle(const std::string& format, Action action, long long emptyKBytes) :
  format(format),
  action(action), 
  emptyKBytes(emptyKBytes)
{
}

NukedirStyle::NukedirStyle(const std::vector<std::string>& toks)   
{
  format = toks[0];
  std::string action = util::ToLowerCopy(toks[1]);
  if (action == "deleteall") action = DeleteAll;
  else if (action == "deletefiles") action = DeleteFiles;
  else if (action == "keep") action = Keep;
  else throw boost::bad_lexical_cast();
  emptyKBytes = ParseSize(toks[2]);
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
    throw boost::bad_lexical_cast();
  target = toks[3];
  if (toks.size() == 5) arguments = toks[4];
}

Cscript::Cscript(const std::vector<std::string>& toks)   
{
  command = util::ToUpperCopy(toks[0]);
  std::string when = util::ToLowerCopy(toks[1]);
  if (when == "pre") type = Type::Pre;
  else if (when == "post") type = Type::Post;
  else throw boost::bad_lexical_cast();
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
    maximum(boost::posix_time::seconds(boost::lexical_cast<long>(toks[0]))),
    minimum(boost::posix_time::seconds(boost::lexical_cast<long>(toks[1]))),
    timeout(boost::posix_time::seconds(boost::lexical_cast<long>(toks[2])))
  {
    if (timeout.total_seconds() < 1 || minimum.total_seconds() < 1 || maximum.total_seconds() < 1)
      throw boost::bad_lexical_cast();
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
  database(toks.size() >= 3 && boost::lexical_cast<long>(toks[2]))
{
  if (database < 0) throw boost::bad_lexical_cast();
}

TransferLog::TransferLog(const std::string& name, const std::vector<std::string>& toks) :
  Log(name, toks),
  uploads(YesNoToBoolean(toks[3])),
  downloads(YesNoToBoolean(toks[4]))
{
}

}