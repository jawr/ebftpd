#include <stdlib.h>
#include "cfg/setting.hpp"
#include "cfg/exception.hpp"
#include "util/string.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>

namespace cfg { namespace setting
{

void TlsCertificate::Save(std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
};

// glftpd 

void AsciiDownloads::Save(std::vector<std::string>& toks)
{
  size = (toks.at(0) == "*") ? -1 : boost::lexical_cast<int>(toks.at(0));
  toks.erase(toks.begin());
  masks = toks;
}

void Shutdown::Save(std::vector<std::string>& toks)
{
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void FreeSpace::Save(std::vector<std::string>& toks)
{
  amount = boost::lexical_cast<int>(toks.at(0));
}

void UseDirSize::Save(std::vector<std::string>& toks)
{
  unit = (char)toks.at(0)[0];
  toks.erase(toks.begin());
  for (std::vector<std::string>::iterator it = toks.begin(); it != toks.end();
    ++it)
  {
    paths.push_back(fs::Path((*it)));
  }
}

void Timezone::Save(std::vector<std::string>& toks)
{
  hours = boost::lexical_cast<int>(toks.at(0));
}

void ColorMode::Save(std::vector<std::string>& toks)
{
  use = util::string::BoolLexicalCast(toks.at(0));
}

void SitenameLong::Save(std::vector<std::string>& toks)
{
  name = toks.at(0);
}

void SitenameShort::Save(std::vector<std::string>& toks)
{
  name = toks.at(0);
}

void LoginPrompt::Save(std::vector<std::string>& toks)
{
  arg = toks.at(0);
}

void Rootpath::Save(std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
}

void ReloadConfig::Save(std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
}

void Master::Save(std::vector<std::string>& toks)
{
  users = toks;
}

void SecureIp::Save(std::vector<std::string>& toks)
{
  minFields = boost::lexical_cast<int>(toks.at(0));
  allowHostname = util::string::BoolLexicalCast(toks.at(1));
  needIdent = util::string::BoolLexicalCast(toks.at(2));
  toks.erase(toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void SecurePass::Save(std::vector<std::string>& toks)
{
  mask = toks.at(0);
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void Datapath::Save(std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
}

void PwdPath::Save(std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
}

void GrpPath::Save(std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
}

void BotscriptPath::Save(std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
}

void BouncerIp::Save(std::vector<std::string>& toks)
{
  addrs = toks;
}

void SpeedLimit::Save(std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
  dlLimit = boost::lexical_cast<long>(toks.at(1));
  ulLimit = boost::lexical_cast<long>(toks.at(2));
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));                                    
}

void SimXfers::Save(std::vector<std::string>& toks)
{
  maxDownloads = boost::lexical_cast<unsigned int>(toks.at(0));
  maxUploads = boost::lexical_cast<unsigned int>(toks.at(1));
}

void CalcCrc::Save(std::vector<std::string>& toks)   
{
  masks = toks;
}

void Xdupe::Save(std::vector<std::string>& toks)   
{
  masks = toks;
}

void MmapAmount::Save(std::vector<std::string>& toks)   
{
  size = boost::lexical_cast<int>(toks.at(0));
}

void DlSendfile::Save(std::vector<std::string>& toks)   
{
  size = boost::lexical_cast<int>(toks.at(0));
}

void UlBufferedForce::Save(std::vector<std::string>& toks)   
{
  size = boost::lexical_cast<int>(toks.at(0));
}

void MinHomedir::Save(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
}

void ValidIp::Save(std::vector<std::string>& toks)   
{
  ips = toks;
}

void ActiveAddr::Save(std::vector<std::string>& toks)   
{
  addr = toks.at(0);
}

void PasvAddr::Save(std::vector<std::string>& toks)   
{
  addr = toks.at(0);
  if (toks.size() > 1) nat = true;
  else nat = false;
}

void Ports::Save(std::vector<std::string>& toks)   
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

void AllowFxp::Save(std::vector<std::string>& toks)   
{
  downloads = util::string::BoolLexicalCast(toks.at(0));
  uploads   = util::string::BoolLexicalCast(toks.at(1));
  logging   = util::string::BoolLexicalCast(toks.at(2));
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void WelcomeMsg::Save(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " ")); 
}

void GoodbyeMsg::Save(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " ")); 
}

void Newsfile::Save(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " ")); 
}

void Banner::Save(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
}

void Alias::Save(std::vector<std::string>& toks)   
{
  alias = toks.at(0);
  path = fs::Path(toks.at(1));
}

void Cdpath::Save(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
}

void IgnoreType::Save(std::vector<std::string>& toks)   
{
  masks = toks;
}

void Right::Save(std::vector<std::string>& toks)
{
  path = toks.at(0);
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void StatSection::Save(std::vector<std::string>& toks)   
{
  keyword = toks.at(0);
  path = toks.at(1);
  seperateCredits = util::string::BoolLexicalCast(toks.at(2));
}

void PathFilter::Save(std::vector<std::string>& toks)   
{
  group = toks.at(0);
  path = toks.at(1);
  toks.erase(toks.begin(), toks.begin()+2);
  filters = toks;
}

void MaxUsers::Save(std::vector<std::string>& toks)   
{
  maxUsers = boost::lexical_cast<int>(toks.at(0));
  maxExemptUsers = boost::lexical_cast<int>(toks.at(1));
}

void MaxUstats::Save(std::vector<std::string>& toks)   
{
  maxResults = boost::lexical_cast<int>(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " ")); 
}

void MaxGstats::Save(std::vector<std::string>& toks)
{
  maxResults = boost::lexical_cast<int>(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " ")); 
}

void BannedUsers::Save(std::vector<std::string>& toks)   
{
  users = toks;
}

void ShowDiz::Save(std::vector<std::string>& toks)   
{
  filename = toks.at(0);
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void ShowTotals::Save(std::vector<std::string>& toks)   
{
  maxLines = (toks.at(0) == "*") ? -1 : boost::lexical_cast<int>(toks.at(0));
  toks.erase(toks.begin());
  paths = toks;
}

void DlIncomplete::Save(std::vector<std::string>& toks)   
{
  enabled = util::string::BoolLexicalCast(toks.at(0));
}

void FileDlCount::Save(std::vector<std::string>& toks)   
{
  enabled = util::string::BoolLexicalCast(toks.at(0));
}

void DupeCheck::Save(std::vector<std::string>& toks)   
{
  days = boost::lexical_cast<int>(toks.at(0));
  ignoreCase = util::string::BoolLexicalCast(toks.at(1));
}

void Script::Save(std::vector<std::string>& toks)   
{
  script = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  masks = toks;
}

void IdleCommands::Save(std::vector<std::string>& toks)   
{
  commands = toks;
}

void TotalUsers::Save(std::vector<std::string>& toks)   
{
  limit = boost::lexical_cast<int>(toks.at(0));
}

void Lslong::Save(std::vector<std::string>& toks)   
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

void HiddenFiles::Save(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  masks = toks;
}

void Noretrieve::Save(std::vector<std::string>& toks)   
{
  masks = toks;
}

void Tagline::Save(std::vector<std::string>& toks)   
{
  tagline = toks.at(0);
}

void Email::Save(std::vector<std::string>& toks)   
{
  email = toks.at(0);
}

void MultiplierMax::Save(std::vector<std::string>& toks)   
{
  max = boost::lexical_cast<int>(toks.at(0));
}

void Oneliners::Save(std::vector<std::string>& toks)   
{
  max = boost::lexical_cast<int>(toks.at(0));                                                       
}

void Requests::Save(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  max = boost::lexical_cast<int>(toks.at(1));
}

void Lastonline::Save(std::vector<std::string>& toks)   
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

void EmptyNuke::Save(std::vector<std::string>& toks)   
{
  amount = boost::lexical_cast<int>(toks.at(0));
}

void Nodupecheck::Save(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
}

void Creditcheck::Save(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  ratio = boost::lexical_cast<int>(toks.at(1));
  toks.erase(toks.begin(), toks.begin()+2);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void Creditloss::Save(std::vector<std::string>& toks)   
{
  multiplier = boost::lexical_cast<int>(toks.at(0));
  allowLeechers = util::string::BoolLexicalCast(toks.at(1));
  path = fs::Path(toks.at(2));
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void NukedirStyle::Save(std::vector<std::string>& toks)   
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

void Hideuser::Save(std::vector<std::string>& toks)   
{
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void Privgroup::Save(std::vector<std::string>& toks)   
{
  group = toks.at(0);
  description = toks.at(1);
}

void Msgpath::Save(std::vector<std::string>& toks)   
{
  path = toks.at(0);
  file = fs::Path(toks.at(1));
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void Privpath::Save(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

void SiteCmd::Save(std::vector<std::string>& toks)   
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

void MaxSitecmdLines::Save(std::vector<std::string>& toks)   
{
  max = boost::lexical_cast<int>(toks.at(0));
}

void Cscript::Save(std::vector<std::string>& toks)   
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
