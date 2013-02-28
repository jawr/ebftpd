#include <sstream>
#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
#include "cfg/setting.hpp"
#include "cfg/error.hpp"
#include "util/string.hpp"
#include "cfg/util.hpp"

namespace cfg
{

Database::Database() :
  name("ebftpd"), 
  address("localhost"), 
  port(27017)
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

PathFilter::PathFilter() :
  regex(new boost::regex("^[[\\]A-Za-z0-9_'()[:space:]][[\\]A-Za-z0-9_.'()[:space:]-]+$")),
  acl("*")
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

Requests::Requests(const std::vector<std::string>& toks)   
{
  path = toks[0];
  max = boost::lexical_cast<int>(toks[1]);
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

NukedirStyle::NukedirStyle() :
  action(Keep), 
  emptyKBytes(ParseSize("1M"))
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
  command = util::ToUpperCopy(toks[0]);
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

  static const std::unique_ptr<IdleTimeoutImpl> defaults;

  IdleTimeoutImpl() :
    maximum(defaults->maximum), minimum(defaults->minimum),
    timeout(defaults->timeout)
  { }

  IdleTimeoutImpl(const boost::posix_time::seconds& maximum,
                  const boost::posix_time::seconds& minimum,
                  const boost::posix_time::seconds& timeout) :
    maximum(maximum), minimum(minimum),
    timeout(timeout)
  { }

  IdleTimeoutImpl(const std::vector<std::string>& toks) :
    maximum(defaults->maximum),
    minimum(defaults->minimum),
    timeout(defaults->timeout)
  {
    namespace pt = boost::posix_time;
    timeout = pt::seconds(boost::lexical_cast<long>(toks[0]));
    minimum = pt::seconds(boost::lexical_cast<long>(toks[1]));
    maximum = pt::seconds(boost::lexical_cast<long>(toks[2]));
    
    if (timeout.total_seconds() < 1 || minimum.total_seconds() < 1 || maximum.total_seconds() < 1)
      throw boost::bad_lexical_cast();
    if (minimum >= maximum)
      throw ConfigError("Mnimum must be smaller than maximum in idle_timeout");
    if (timeout < minimum || timeout > maximum)
      throw ConfigError("Default timeout must be larger than or equal to minimum "
                        "and smaller than or equal to maximum in idle_timeout");
  }
};

const std::unique_ptr<IdleTimeoutImpl> IdleTimeoutImpl::defaults(new IdleTimeoutImpl(
  boost::posix_time::seconds(7200),
  boost::posix_time::seconds(1),
  boost::posix_time::seconds(900)
));

IdleTimeout::IdleTimeout() :
  pimpl(new IdleTimeoutImpl())
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