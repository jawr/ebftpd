#include <stdlib.h>
#include "cfg/setting.hpp"
#include "cfg/exception.hpp"
#include "util/string.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>

#include <iostream>

namespace cfg { namespace setting
{

// glftpd 

AsciiDownloads::AsciiDownloads(std::vector<std::string>& toks)
{
  size = (toks.at(0) == "*") ? -1 : boost::lexical_cast<int>(toks.at(0));
  toks.erase(toks.begin());
  masks = toks;
}

UseDirSize::UseDirSize(std::vector<std::string>& toks)
{
  unit = (char)toks.at(0)[0];
  toks.erase(toks.begin());
  for (std::vector<std::string>::iterator it = toks.begin(); it != toks.end();
    ++it)
  {
    paths.push_back(fs::Path((*it)));
  }
}

SecureIp::SecureIp(std::vector<std::string>& toks)
{
  minFields = boost::lexical_cast<int>(toks.at(0));
  allowHostname = util::string::BoolLexicalCast(toks.at(1));
  needIdent = util::string::BoolLexicalCast(toks.at(2));
  toks.erase(toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

SecurePass::SecurePass(std::vector<std::string>& toks)
{
  mask = toks.at(0);
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

SpeedLimit::SpeedLimit(std::vector<std::string>& toks)
{
  path = fs::Path(toks.at(0));
  dlLimit = boost::lexical_cast<long>(toks.at(1));
  ulLimit = boost::lexical_cast<long>(toks.at(2));
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));                                    
}

SimXfers::SimXfers(std::vector<std::string>& toks)
{
  maxDownloads = boost::lexical_cast<unsigned int>(toks.at(0));
  maxUploads = boost::lexical_cast<unsigned int>(toks.at(1));
}

PasvAddr::PasvAddr(std::vector<std::string>& toks)   
{
  addr = toks.at(0);
  if (toks.size() > 1) nat = true;
  else nat = false;
}

Ports::Ports(std::vector<std::string>& toks)   
{
  std::vector<std::string> temp;
  for (std::vector<std::string>::iterator it = toks.begin(); it != toks.end();
    ++it)
  {
    temp.clear();
    boost::split(temp, *it, boost::is_any_of("-"));
    if (temp.size() > 2) throw cfg::ConfigError("Invalid port range.");
    int from = boost::lexical_cast<int>(temp.at(0));
    int to = from;
    if (temp.size() > 1) to = boost::lexical_cast<int>(temp.at(1));
    if (to < from)
      throw cfg::ConfigError("To port lower than from port in port range.");
    if (to < 1024 || from < 1024 || to > 65535 || from > 65535)
      throw cfg::ConfigError("Invalid to port number in port range.");
    ranges.push_back(PortRange(from, to));
  }
}

AllowFxp::AllowFxp(std::vector<std::string>& toks)   
{
  downloads = util::string::BoolLexicalCast(toks.at(0));
  uploads   = util::string::BoolLexicalCast(toks.at(1));
  logging   = util::string::BoolLexicalCast(toks.at(2));
  toks.erase(toks.begin(), toks.begin() + 3);
  if (toks.empty()) throw cfg::ConfigError("Missing ACL parameter on allow_fxp");
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

Alias::Alias(std::vector<std::string>& toks)   
{
  name = toks.at(0);
  path = fs::Path(toks.at(1));
}

Right::Right(std::vector<std::string>& toks)
{
  path = toks.at(0);
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

StatSection::StatSection(std::vector<std::string>& toks)   
{
  keyword = toks.at(0);
  path = toks.at(1);
  seperateCredits = util::string::BoolLexicalCast(toks.at(2));
}

PathFilter::PathFilter(std::vector<std::string>& toks)   
{
  group = toks.at(0);
  path = toks.at(1);
  toks.erase(toks.begin(), toks.begin()+2);
  filters = toks;
}

MaxUsers::MaxUsers(std::vector<std::string>& toks)   
{
  users = boost::lexical_cast<int>(toks.at(0));
  exemptUsers = boost::lexical_cast<int>(toks.at(1));
}

ACLInt::ACLInt(std::vector<std::string>& toks)   
{
  arg = boost::lexical_cast<int>(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " ")); 
}

ShowTotals::ShowTotals(std::vector<std::string>& toks)   
{
  maxLines = (toks.at(0) == "*") ? -1 : boost::lexical_cast<int>(toks.at(0));
  toks.erase(toks.begin());
  paths = toks;
}

DupeCheck::DupeCheck(std::vector<std::string>& toks)   
{
  days = boost::lexical_cast<int>(toks.at(0));
  ignoreCase = util::string::BoolLexicalCast(toks.at(1));
}

Script::Script(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  masks = toks;
}

Lslong::Lslong(std::vector<std::string>& toks)   
{
  options = toks[0];
  if (options[0] == '-') options.erase(0, 1);
  if (toks.size() == 1) return;
  
  try
  {
    maxRecursion = boost::lexical_cast<int>(toks[1]);
    toks.pop_back();
  }
  catch (const boost::bad_lexical_cast& e)
  {
    throw cfg::ConfigError(
      "Invalid number for optional lslong recrusion paramter");
  }
}

HiddenFiles::HiddenFiles(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  masks = toks;
}

Requests::Requests(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  max = boost::lexical_cast<int>(toks.at(1));
}

Lastonline::Lastonline(std::vector<std::string>& toks)   
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
      throw cfg::ConfigError("Invalid lastonline param.");
      break;
  }
  if (toks.size() > 1)
    max = boost::lexical_cast<int>(toks.at(1));
  else
    max = 10;
}

Creditcheck::Creditcheck(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  ratio = boost::lexical_cast<int>(toks.at(1));
  toks.erase(toks.begin(), toks.begin()+2);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

Creditloss::Creditloss(std::vector<std::string>& toks)   
{
  multiplier = boost::lexical_cast<int>(toks.at(0));
  allowLeechers = util::string::BoolLexicalCast(toks.at(1));
  path = fs::Path(toks.at(2));
  toks.erase(toks.begin(), toks.begin()+3);
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

NukedirStyle::NukedirStyle(std::vector<std::string>& toks)   
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
      throw cfg::ConfigError("Invalid nukedir_style param.");
      break;
  }
  minBytes = boost::lexical_cast<int>(toks.at(2));
}

Privgroup::Privgroup(std::vector<std::string>& toks)   
{
  group = toks.at(0);
  description = toks.at(1);
}

Msgpath::Msgpath(std::vector<std::string>& toks)   
{
  path = toks.at(0);
  file = fs::Path(toks.at(1));
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

Privpath::Privpath(std::vector<std::string>& toks)   
{
  path = fs::Path(toks.at(0));
  toks.erase(toks.begin());
  acl = acl::ACL::FromString(boost::algorithm::join(toks, " "));
}

SiteCmd::SiteCmd(std::vector<std::string>& toks)   
{
  command = toks.at(0);
  if (toks.at(1) == "EXEC") type = EXEC;
  else if (toks.at(1) == "TEXT") type = TEXT;
  else if (toks.at(1) == "IS") type = IS;
  else
    throw cfg::ConfigError("Invalid site_cmd param.");
  script = toks.at(2);
  toks.erase(toks.begin(), toks.begin()+3);
  arguments = toks;
}

Cscript::Cscript(std::vector<std::string>& toks)   
{
  command = toks.at(0);
  std::string when = toks.at(1);
  if (when == "pre") type = PRE;
  else if (when == "post") type = POST;
  else throw cfg::ConfigError("Invalid cscript param.");
  script = fs::Path(toks.at(2));
  
}

// end namespace
}
}
