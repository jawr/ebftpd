#include <stdlib.h>
#include "cfg/setting.hpp"
#include "cfg/exception.hpp"
#include "util/string.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>

namespace cfg { namespace setting
{

void ACL(std::vector<std::string>& toks)
{
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

// glftpd 

void AsciiDownloads::AsciiDownloads(std::vector<std::string>& toks)
{
  size = (toks.at(0) == "*") ? -1 : boost::lexical_cast<int>(toks.at(0));
  toks.erase(toks.begin());
  masks = toks;
}

void UseDirSize::UseDirSize(std::vector<std::string>& toks)
{
  unit = (char)toks.at(0)[0];
  toks.erase(toks.begin());
  for (std::vector<std::string>::iterator it = toks.begin(); it != toks.end();
    ++it)
  {
    paths.push_back(fs::Path((*it)));
  }
}

void Timezone::Timezone(std::vector<std::string>& toks)
{
  hours = boost::lexical_cast<int>(toks.at(0));
}

void SecureIp::SecureIp(std::vector<std::string>& toks)
{
  minFields = boost::lexical_cast<int>(toks.at(0));
  allowHostname = util::string::BoolLexicalCast(toks.at(1));
  needIdent = util::string::BoolLexicalCast(toks.at(2));
  toks.erase(toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void SecurePass::SecurePass(std::vector<std::string>& toks)
{
  mask = toks.at(0);
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void SpeedLimit::SpeedLimit(std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
  dlLimit = boost::lexical_cast<long>(toks.at(1));
  ulLimit = boost::lexical_cast<long>(toks.at(2));
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));                                    
}

void SimXfers::SimXfers(std::vector<std::string>& toks)
{
  maxDownloads = boost::lexical_cast<unsigned int>(toks.at(0));
  maxUploads = boost::lexical_cast<unsigned int>(toks.at(1));
}

void PasvAddr::PasvAddr(std::vector<std::string>& toks)   
{
  addr = toks.at(0);
  if (toks.size() > 1) nat = true;
  else nat = false;
}

void Ports::Ports(std::vector<std::string>& toks)   
{
  std::vector<std::string> temp;
  for (std::vector<std::string>::iterator it = toks.begin(); it != toks.end();
    ++it)
  {
    temp.clear();
    boost::split(temp, (*it), boost::is_any_of("-"));
    int from = boost::lexical_cast<int>(temp.at(0));
    int to = from;
    if (temp.size() > 1) to = boost::lexical_cast<int>(temp.at(1));
    ranges.push_back(PortRange(from, to));
  }
}

void AllowFxp::AllowFxp(std::vector<std::string>& toks)   
{
  downloads = util::string::BoolLexicalCast(toks.at(0));
  uploads   = util::string::BoolLexicalCast(toks.at(1));
  logging   = util::string::BoolLexicalCast(toks.at(2));
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void Alias::Alias(std::vector<std::string>& toks)   
{
  alias = toks.at(0);
  path = fs::Path(toks.at(1));
}

void Right::Right(std::vector<std::string>& toks)
{
  path = toks.at(0);
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void StatSection::StatSection(std::vector<std::string>& toks)   
{
  keyword = toks.at(0);
  path = toks.at(1);
  seperateCredits = util::string::BoolLexicalCast(toks.at(2));
}

void PathFilter::PathFilter(std::vector<std::string>& toks)   
{
  group = toks.at(0);
  path = toks.at(1);
  toks.erase(toks.begin(), toks.begin()+2);
  filters = toks;
}

void MaxUsers::MaxUsers(std::vector<std::string>& toks)   
{
  maxUsers = boost::lexical_cast<int>(toks.at(0));
  maxExemptUsers = boost::lexical_cast<int>(toks.at(1));
}

void ACLInt(std::vector<std::string>& toks)   
{
  maxResults = boost::lexical_cast<int>(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " ")); 
}

void ShowTotals::ShowTotals(std::vector<std::string>& toks)   
{
  maxLines = (toks.at(0) == "*") ? -1 : boost::lexical_cast<int>(toks.at(0));
  toks.erase(toks.begin());
  paths = toks;
}

void DupeCheck::DupeCheck(std::vector<std::string>& toks)   
{
  days = boost::lexical_cast<int>(toks.at(0));
  ignoreCase = util::string::BoolLexicalCast(toks.at(1));
}

void Script::Script(std::vector<std::string>& toks)   
{
  script = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  masks = toks;
}

void Lslong::Lslong(std::vector<std::string>& toks)   
{
  bin = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  if (toks.size() == 0) return;
  try
  {
    maxRecursion = boost::lexical_cast<int>(toks.back());
    toks.pop_back();
  }
  catch (const boost::bad_lexical_cast& e)
  {
    maxRecursion = 2;
  }
  options = toks;
}

void HiddenFiles::HiddenFiles(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  masks = toks;
}

void Requests::Requests(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  max = boost::lexical_cast<int>(toks.at(1));
}

void Lastonline::Lastonline(std::vector<std::string>& toks)   
{
  int i = boost::lexical_cast<int>(toks.at(0));
  switch (i) 
  {
    case 0:
      type = lastonline::ALL;
      break;
    case 1:
      type = lastonline::TIMEOUT;
      break;
    case 2:
      type = lastonline::ALL_WITH_ACTIVITY;
      break;
    default:
      throw cfg::ConfigError("Invalid lastonline param.");
      break;
  }
  if (toks.size() > 1)
    max = boost::lexical_cast<int>(toks.at(1));
  else
    max = 10;
}

void Creditcheck::Creditcheck(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  ratio = boost::lexical_cast<int>(toks.at(1));
  toks.erase(toks.begin(), toks.begin()+2);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void Creditloss::Creditloss(std::vector<std::string>& toks)   
{
  multiplier = boost::lexical_cast<int>(toks.at(0));
  allowLeechers = util::string::BoolLexicalCast(toks.at(1));
  path = fs::Path(toks.at(2));
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void NukedirStyle::NukedirStyle(std::vector<std::string>& toks)   
{
  format = toks.at(0);
  int i = boost::lexical_cast<int>(toks.at(1));
  switch (i)
  {
    case 0:
      type = nukedirstyle::DELETE_ALL;
      break;
    case 1:
      type = nukedirstyle::DELETE_FILES;
      break;
    case 2:
      type = nukedirstyle::KEEP;
      break;
    default:
      throw cfg::ConfigError("Invalid nukedir_style param.");
      break;
  }
  minBytes = boost::lexical_cast<int>(toks.at(2));
}

void Privgroup::Privgroup(std::vector<std::string>& toks)   
{
  group = toks.at(0);
  description = toks.at(1);
}

void Msgpath::Msgpath(std::vector<std::string>& toks)   
{
  path = toks.at(0);
  file = fs::Path(toks.at(1));
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void Privpath::Privpath(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void SiteCmd::SiteCmd(std::vector<std::string>& toks)   
{
  command = toks.at(0);
  if (toks.at(1) == "EXEC") type = sitecmd::EXEC;
  else if (toks.at(1) == "TEXT") type = sitecmd::TEXT;
  else if (toks.at(1) == "IS") type = sitecmd::IS;
  else
    throw cfg::ConfigError("Invalid site_cmd param.");
  script = toks.at(2);
  toks.erase(toks.begin(), toks.begin()+3);
  arguments = toks;
}

void Cscript::Cscript(std::vector<std::string>& toks)   
{
  command = toks.at(0);
  std::string when = toks.at(1);
  if (when == "pre") type = cscript::PRE;
  else if (when == "post") type = cscript::POST;
  else throw cfg::ConfigError("Invalid cscript param.");
  script = fs::Path(toks.at(2));
  
}

// end namespace
}
}
