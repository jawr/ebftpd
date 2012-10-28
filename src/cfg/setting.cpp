#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include "cfg/setting.hpp"
#include "cfg/exception.hpp"
#include "util/string.hpp"

#include <iostream>

namespace cfg { namespace setting
{

const boost::posix_time::seconds IdleTimeout::defaultMaximum(7200);
const boost::posix_time::seconds IdleTimeout::defaultMinimum(1);
const boost::posix_time::seconds IdleTimeout::defaultTimeout(900);

// glftpd 

AsciiDownloads::AsciiDownloads(const std::vector<std::string>& toks) :
  size(-1)
{
  try
  {
    size = boost::lexical_cast<int>(toks[0]);
  }
  catch (const boost::bad_lexical_cast&) { }
  if (size == 0) size = -1;
  masks.assign(toks.begin() + 1, toks.end());
}

SecureIp::SecureIp(std::vector<std::string> toks)
{
  int numOctets = boost::lexical_cast<int>(toks.at(0));
  if (numOctets < 0) throw boost::bad_lexical_cast();
  bool isHostname = util::string::BoolLexicalCast(toks.at(1));
  bool hasIdent = util::string::BoolLexicalCast(toks.at(2));
  strength = acl::IPStrength(numOctets, isHostname, hasIdent);
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

SecurePass::SecurePass(std::vector<std::string> toks) :
  strength(toks[0])
{
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

SpeedLimit::SpeedLimit(std::vector<std::string> toks)
{
  path = fs::Path(toks.at(0));
  dlLimit = boost::lexical_cast<long>(toks.at(1));
  ulLimit = boost::lexical_cast<long>(toks.at(2));
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));                                    
}

SimXfers::SimXfers(std::vector<std::string> toks)
{
  maxDownloads = boost::lexical_cast<unsigned int>(toks.at(0));
  maxUploads = boost::lexical_cast<unsigned int>(toks.at(1));
}

PasvAddr::PasvAddr(const std::vector<std::string>& toks)   
{
  addr = toks.at(0);
  if (toks.size() > 1) nat = util::string::BoolLexicalCast(toks[1]);
}

Ports::Ports(const std::vector<std::string>& toks)   
{
  std::vector<std::string> temp;
  for (const auto& token : toks)
  {
    temp.clear();
    boost::split(temp, token, boost::is_any_of("-"));
    if (temp.size() > 2) throw cfg::ConfigError("Invalid port range.");
    int from = boost::lexical_cast<int>(temp.at(0));
    int to = from;
    if (temp.size() > 1) to = boost::lexical_cast<int>(temp.at(1));
    if (to < from)
      throw cfg::ConfigError("To port lower than from port in port range.");
    if (to < 1024 || from < 1024 || to > 65535 || from > 65535)
      throw cfg::ConfigError("Invalid to port number in port range.");
    ranges.emplace_back(from, to);
  }
}

AllowFxp::AllowFxp(std::vector<std::string> toks)   
{
  downloads = util::string::BoolLexicalCast(toks.at(0));
  uploads   = util::string::BoolLexicalCast(toks.at(1));
  logging   = util::string::BoolLexicalCast(toks.at(2));
  toks.erase(toks.begin(), toks.begin() + 3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

Alias::Alias(const std::vector<std::string>& toks)   
{
  name = toks.at(0);
  path = fs::Path(toks.at(1));
}

Right::Right(std::vector<std::string> toks)
{
  path = toks.at(0);
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

StatSection::StatSection(const std::vector<std::string>& toks)   
{
  keyword = toks.at(0);
  path = toks.at(1);
  seperateCredits = util::string::BoolLexicalCast(toks.at(2));
}

PathFilter::PathFilter(std::vector<std::string> toks)   
{
  group = toks.at(0);
  path = toks.at(1);
  toks.erase(toks.begin(), toks.begin()+2);
  filters = toks;
}

MaxUsers::MaxUsers(const std::vector<std::string>& toks)   
{
  users = boost::lexical_cast<int>(toks.at(0));
  exemptUsers = boost::lexical_cast<int>(toks.at(1));
}

ACLInt::ACLInt(std::vector<std::string> toks)   
{
  arg = boost::lexical_cast<int>(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " ")); 
}

ShowTotals::ShowTotals(std::vector<std::string> toks)   
{
  maxLines = (toks.at(0) == "*") ? -1 : boost::lexical_cast<int>(toks.at(0));
  if (maxLines < -1) throw boost::bad_lexical_cast();
  toks.erase(toks.begin());
  paths = toks;
}

DupeCheck::DupeCheck(const std::vector<std::string>& toks)   
{
  days = boost::lexical_cast<int>(toks.at(0));
  if (days < 0) throw boost::bad_lexical_cast();
  ignoreCase = util::string::BoolLexicalCast(toks.at(1));
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
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  masks = toks;
}

Requests::Requests(const std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  max = boost::lexical_cast<int>(toks.at(1));
}

Lastonline::Lastonline(const std::vector<std::string>& toks) :
  max(10)
{
  int i = boost::lexical_cast<int>(toks.at(0));
  switch (i) 
  {
    case 0:
      type = ALL;
      break;
    case 1:
      type = TIMEOUT;
      break;
    case 2:
      type = ALL_WITH_ACTIVITY;
      break;
    default:
      throw cfg::ConfigError("Invalid lastonline parameter");
      break;
  }
  
  if (toks.size() > 1)
    max = boost::lexical_cast<int>(toks.at(1));
}

Creditcheck::Creditcheck(std::vector<std::string> toks)   
{
  path = fs::Path(toks.at(0));
  ratio = boost::lexical_cast<int>(toks.at(1));
  toks.erase(toks.begin(), toks.begin()+2);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

Creditloss::Creditloss(std::vector<std::string> toks)   
{
  multiplier = boost::lexical_cast<int>(toks.at(0));
  allowLeechers = util::string::BoolLexicalCast(toks.at(1));
  path = fs::Path(toks.at(2));
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

NukedirStyle::NukedirStyle(const std::vector<std::string>& toks)   
{
  format = toks.at(0);
  int i = boost::lexical_cast<int>(toks.at(1));
  switch (i)
  {
    case 0:
      type = DELETE_ALL;
      break;
    case 1:
      type = DELETE_FILES;
      break;
    case 2:
      type = KEEP;
      break;
    default:
      throw cfg::ConfigError("Invalid nukedir_style parameter");
      break;
  }
  minBytes = boost::lexical_cast<int>(toks.at(2));
}

Privgroup::Privgroup(const std::vector<std::string>& toks)   
{
  group = toks.at(0);
  description = toks.at(1);
}

Msgpath::Msgpath(const std::vector<std::string>& toks)   
{
  path = toks.at(0);
  file = fs::Path(toks.at(1));
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

Privpath::Privpath(std::vector<std::string> toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

SiteCmd::SiteCmd(const std::vector<std::string>& toks)   
{
  command = toks.at(0);
  if (toks.at(1) == "EXEC") type = EXEC;
  else if (toks.at(1) == "TEXT") type = TEXT;
  else if (toks.at(1) == "IS") type = IS;
  else
    throw cfg::ConfigError("Invalid site_cmd parameter");
  script = toks.at(2);
  arguments.assign(toks.begin() + 4, toks.end());
}

Cscript::Cscript(const std::vector<std::string>& toks)   
{
  command = toks.at(0);
  std::string when = toks.at(1);
  if (when == "pre") type = PRE;
  else if (when == "post") type = POST;
  else throw cfg::ConfigError("Invalid cscript parameter");
  script = fs::Path(toks.at(2));
}

IdleTimeout::IdleTimeout(const std::vector<std::string>& toks) :
  maximum(defaultMaximum),
  minimum(defaultMinimum),
  timeout(defaultTimeout)
{
  namespace pt = boost::posix_time;
  timeout = pt::seconds(boost::lexical_cast<long>(toks[0]));
  minimum = pt::seconds(boost::lexical_cast<long>(toks[1]));
  maximum = pt::seconds(boost::lexical_cast<long>(toks[2]));
  
  if (timeout.total_seconds() < 1 || minimum.total_seconds() < 1 || maximum.total_seconds() < 1)
    throw ConfigError("Times in idle_timeout must be larger than zero");
  if (minimum >= maximum)
    throw ConfigError("Mnimum must be smaller than maximum in idle_timeout");
  if (timeout < minimum || timeout > maximum)
    throw ConfigError("Default timeout must be larger than or equal to minimum "
                      "and smaller than or equal to maximum in idle_timeout");
}

// end namespace
}
}
