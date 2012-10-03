#include "cfg/setting.hpp"
#include "util/string.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>

namespace cfg { namespace setting
{

virtual void AsciiDownloads::Save(const std::vector<std::string>& toks)
{
  size= (args.at(0) == "*") ? -1 : boost::lexical_cast<int>(args.at(0));
  toks.erase(toks.begin());
  masks = toks;
}

virtual void Shutdown::Save(const std::vector<std::string>& toks)
{
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

virtual void FreeSpace::Save(const std::vector<std::string>& toks)
{
  amount = boost::lexical_cast<int>(args.at(0));
}

virtual void UseDirSize::Save(const std::vector<std::string>& toks)
{
  unit = toks.at(0).c_str();
  toks.erase(toks.begin());
  for (std::vector<std::string>::iterator it = toks.begin(); it != toks.end();
    ++it)
  {
    paths.push_back(fs::Path((*it)));
  }
}

virtual void Timezone::Save(const std::vector<std::string>& toks)
{
  hours = boost::lexical_cast<int>(toks.at(0));
}

virtual void ColorMode::Save(const std::vector<std::string>& toks)
{
  bool = util::string::BoolLexicalCast(toks.at(0));
}

virtual void SitenameLong::Save(const std::vector<std::string>& toks)
{
  name = toks.at(0);
}

virtual void SitenameShort::Save(const std::vector<std::string>& toks)
{
  name = toks.at(0);
}

virtual void LoginPrompt::Save(const std::vector<std::string>& toks)
{
  arg = toks.at(0);
}

virtual void RootPath::Save(const std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
}

virtual void ReloadConfig::Save(const std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
}

virtual void Master::Save(const std::vector<std::string>& toks)
{
  users = toks;
}

virtual void SecureIp::Save(const std::vector<std::string>& toks)
{
  minFields = boost::lexical_cast<int>(toks.at(0));
  allowHostname = util::string::BoolLexicalCast(toks.at(1));
  needIdent = util::string::BoolLexicalCast(toks.at(2));
  toks.erase(toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

virtual void SecurePass::Save(const std::vector<std::string>& toks)
{
  mask = toks.at(0);
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

virtual void DataPath::Save(const std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
}

virtual void PwdPath::Save(const std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
}

virtual void GrpPath::Save(const std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
}

virtual void BotscriptPath::Save(const std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
}

virtual void BouncerIp::Save(const std::vector<std::string>& toks)
{
  addrs = toks;
}

virtual void SpeedLimit::Save(const std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
  dlLimit = boost::lexical_cast<long>(toks.at(1));
  ulLimit = boost::lexical_cast<long>(toks.at(2));
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));                                    
}

virtual void SimXfers::Save(const std::vector<std::string>& toks)
{
  maxDownloads = boost::lexical_cast<unsigned int>(toks.at(0));
  maxUploads = boost::lexical_cast<unsigned int>(toks.at(1));
}

virtual void CalcCrc::Save (const std::vector<std::string>& toks)   
{
  masks = toks;
}

virtual void Xdupe::Save (const std::vector<std::string>& toks)   
{
  masks = toks;
}

virtual void MmapAmount::Save (const std::vector<std::string>& toks)   
{
  size = boost::lexical_cast<int>(toks.at(0));
}

virtual void DlSendfile::Save (const std::vector<std::string>& toks)   
{
  size = boost::lexical_cast<int>(toks.at(0));
}

virtual void UlBufferedForce::Save (const std::vector<std::string>& toks)   
{
  size = boost::lexical_cast<int>(toks.at(0));
}

virtual void MinHomedir::Save (const std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
}

virtual void ValidIp::Save (const std::vector<std::string>& toks)   
{
  ips = toks;
}

virtual void ActiveAddr::Save (const std::vector<std::string>& toks)   
{
  addr = toks.at(0);
}

virtual void PasvAddr::Save (const std::vector<std::string>& toks)   
{
  addr = toks.at(0);
  if (toks.size() > 1) nat = true;
  else nat = false;
}

virtual void Ports::Save (const std::vector<std::string>& toks)   
{
  std::vector<std::string> templ
  for (std::vector<std::string>::iterator it = toks.begin(); it != toks.end();
    ++it)
  {
    templ.clear();
    boost::split(temp, (*it), boost::is_any_of("-"));
    int from = boost::lexical_cast<int>(temp.at(0));
    int to = from;
    if (temp.size() > 1) to = boost::lexical_cast<int>(temp.at(1));
    ranges.push_back(PortRange(from, to));
  }
}

virtual void AllowFxp::Save (const std::vector<std::string>& toks)   
{
  downloads = util::string::BoolLexicalCast(toks.at(0));
  uploads   = util::string::BoolLexicalCast(toks.at(1));
  logging   = util::string::BoolLexicalCast(toks.at(2));
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

virtual void WelcomeMsg::Save (const std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " ")); 
}

virtual void GoodbyeMsg::Save (const std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " ")); 
}

virtual void Newsfile::Save (const std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " ")); 
}

virtual void Banner::Save (const std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
}

virtual void Alias::Save (const std::vector<std::string>& toks)   
{
  alias = toks.at(0);
  path = fs::Path(toks.at(1));
}

virtual void Cdpath::Save (const std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
}

virtual void IgnoreType::Save (const std::vector<std::string>& toks)   
{
  masks = toks;
}

virtual void Right::Save(const std::vector<std::string>& toks)
{
  path = toks.at(0);
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

virtual void StatSection::Save (const std::vector<std::string>& toks)   
{
  keyword = toks.at(0);
  path = toks.at(1);
  seperateCredits = util::string::BoolLexicalCast(toks.at(2));
}

virtual void PathFilter::Save (const std::vector<std::string>& toks)   
{
  group = toks.at(0);
  path = toks.at(1);
  toks.erase(toks.begin(), toks.begin()+2);
  filters = toks;
}

virtual void MaxUsers::Save (const std::vector<std::string>& toks)   
{
  maxUsers = boost::lexical_cast<int>(toks.at(0));
  maxExemptUsers = boost::lexical_cast<int>(toks.at(1));
}

virtual void MaxUstats::Save (const std::vector<std::string>& toks)   
{
  maxResults = boost::lexical_cast<int>(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " ")); 
}

virtual void MaxGstats::Save (const std::vector<std::string>& toks)   
{
  maxResults = boost::lexical_cast<int>(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " ")); 
}

virtual void BannedUsers::Save (const std::vector<std::string>& toks)   
{
  users = toks;
}

virtual void ShowDiz::Save (const std::vector<std::string>& toks)   
{
  filename = toks.at(0);
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

virtual void ShowTotals::Save (const std::vector<std::string>& toks)   
{
  maxLines = boost::lexical_cast<int>(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

virtual void DlIncomplete::Save (const std::vector<std::string>& toks)   
{
  enabled = util::string::BoolLexicalCast(toks.at(0));
}

virtual void FileDlCount::Save (const std::vector<std::string>& toks)   
{
  enabled = util::string::BoolLexicalCast(toks.at(0));
}

virtual void DupeCheck::Save (const std::vector<std::string>& toks)   
{
  days = boost::lexical_cast<int>(toks.at(0));
  ignoreCase = util::string::BoolLexicalCast(toks.at(1));
}

virtual void Script::Save (const std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  masks = toks;
}

virtual void IdleCommands::Save (const std::vector<std::string>& toks)   
{
  commands = toks;
}

virtual void TotalUsers::Save (const std::vector<std::string>& toks)   
{
  limit = boost::lexical_cast<int>(toks.at(0));
}

virtual void Lslong::Save (const std::vector<std::string>& toks)   
{
  bin = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  if (toks.size() == 0) return;
  if (isdigit(toks.back())) 
  {
    maxRecursion = boost::lexical_cast<int>(toks.back());
    toks.pop_back();
  }
  else maxRecursion = 2;
  options = toks;
}

virtual void HiddenFile::Save (const std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  masks = toks;
}

virtual void Noretrieve::Save (const std::vector<std::string>& toks)   
{
  masks = toks;
}

virtual void Tagline::Save (const std::vector<std::string>& toks)   
{
  tagline = toks.at(0);
}

virtual void Email::Save (const std::vector<std::string>& toks)   
{
  email = toks.at(0);
}

virtual void MultiplierMax::Save (const std::vector<std::string>& toks)   
{
  max = boost::lexical_cast<int>(toks.at(0));
}

virtual void Oneliners::Save (const std::vector<std::string>& toks)   
{
  max = boost::lexical_cast<int>(toks.at(0));                                                       
}

virtual void Requests::Save (const std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  max = boost::lexical_cast<int>(toks.at(1));
}

virtual void Lastonline::Save (const std::vector<std::string>& toks)   
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
  max = boost::lexical_cast<int>(toks.at(1));
}

virtual void EmptyNuke::Save (const std::vector<std::string>& toks)   
{
  amount = boost::lexical_cast<int>(toks.at(0));
}

virtual void Nodupecheck::Save (const std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
}

virtual void Creditcheck::Save (const std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  ratio = boost::lexical_cast<int>(toks.at(1));
  toks.erase(toks.begin(), toks.begin()+2);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

virtual void Creditloss::Save (const std::vector<std::string>& toks)   
{
  multiplier = boost::lexical_cast<int>(toks.at(0));
  allowLeechers = util::string::BoolLexicalCast(toks.at(1));
  path = fs::Path(toks.at(2));
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

virtual void NukedirStyle::Save (const std::vector<std::string>& toks)   
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
    default:
      throw cfg::ConfigError("Invalid nukedir_style param.");
      break;
  }
  minBytes = boost::lexical_cast<int>(toks.at(2));
}

virtual void Hideuser::Save (const std::vector<std::string>& toks)   
{
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

virtual void Privgroup::Save (const std::vector<std::string>& toks)   
{
  group = toks.at(0);
  description = toks.at(1);
}

virtual void Msgpath::Save (const std::vector<std::string>& toks)   
{
  path = toks.at(0);
  file = fs::Path(toks.at(1));
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

virtual void Privpath::Save (const std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

virtual void SiteCmd::Save (const std::vector<std::string>& toks)   
{
  command = toks.at(0);
  int i = boost::lexical_cast<int>(toks.at(1));
  switch (i)
  {
    case 0:
      type = sitecmd::EXEC;
      break;
    case 1:
      type = sitecmd::TEXT;
      break;
    case 2:
      type = sitecmd::IS;
      break;
    default:
      throw cfg::ConfigError("Invalid site_cmd param.");
      break;
  }
  script = toks.at(2);
  toks.erase(toks.begin(), toks.begin()+3);
  arguments = toks;
}

virtual void MaxSitecmdLines::Save (const std::vector<std::string>& toks)   
{
  max = boost::lexical_cast<int>(toks.at(0));
}

virtual void Cscript::Save (const std::vector<std::string>& toks)   
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
