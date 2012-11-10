#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/function.hpp>                                   
#include "cfg/config.hpp"
#include "cfg/exception.hpp"
#include "cfg/setting.hpp"
#include "util/string.hpp"
#include "logs/logs.hpp"
#include "main.hpp"

namespace cfg
{

int Config::latestVersion = 0;
std::unordered_set<std::string> Config::aclKeywords;

Config::Config(const std::string& configFile) : 
  version(++latestVersion),
  configFile(configFile),
  port(-1),
  defaultFlags("3"),
  freeSpace(100),
  timezone(0),
  sitenameLong("SITE NAME"),
  sitenameShort("SN"),
  dlIncomplete(true),
  totalUsers(20),
  multiplierMax(10),
  oneliners(10),
  emptyNuke(102400),
  maxSitecmdLines(-1) // unlimited
{
  std::string line;
  std::ifstream io(configFile.c_str());
  if (!io) throw ConfigError("Unable to open config file.");

  bool okay = true;
  for (int i = 1; std::getline(io, line); ++i)
  {
    std::string::size_type pos = line.find_first_of('#');
    if (pos != std::string::npos) line.erase(pos);
    if (line.empty()) continue;
    
    try 
    {
      Parse(line);
    } 
    catch (const ConfigError &e) // handle properly
    {
      logs::error << "Error in config at line " << i << ": " << e.Message() << logs::endl;
      okay = false;
    }
  }
  
  if (!okay) throw ConfigError("Errors while parsing config file.");

  SanityCheck();
}

void Config::Parse(std::string line) {
  std::vector<std::string> toks;
  boost::trim(line);
  boost::split(toks, line, boost::is_any_of("\t "), boost::token_compress_on);
  for (auto& token : toks) boost::replace_all(token, "[:space:]", " ");

  if (toks.size() == 0) return;
  std::string opt = toks.at(0);
  if (opt.size() == 0) return;
  
  // remove setting from args
  toks.erase(toks.begin());    
  boost::algorithm::to_lower(opt);

  // update cache for sanity check
  settingsCache[opt]++; 

  if (opt[0] == '-')
  {
    ParameterCheck(opt, toks, 1, -1);
    std::string keyword(opt.substr(1));
    if (aclKeywords.find(keyword) == aclKeywords.end())
      throw ConfigError("Invalid command acl keyword: " + keyword);
    commandACLs.insert(std::make_pair(keyword, 
        acl::ACL::FromString(boost::join(toks, " "))));
  }
  else
  if (boost::starts_with(opt, "custom-"))
  {
    ParameterCheck(opt, toks, 1, -1);
    std::string keyword(opt.substr(7));
    if (std::find_if(siteCmd.begin(), siteCmd.end(), 
        [&](const setting::SiteCmd& sc)
        {
          return boost::to_upper_copy(keyword) == sc.Command();
        }) == siteCmd.end())
    {
      throw ConfigError("Invalid custom command acl keyword: " + keyword);
    }
    commandACLs.insert(std::make_pair(keyword, 
        acl::ACL::FromString(boost::join(toks, " "))));
  }
  else
  if (opt == "database")
  {
    ParameterCheck(opt, toks, 3);
    database = setting::Database(toks);
  }
  else
  if (opt == "sitepath")
  {
    ParameterCheck(opt, toks, 1);
    sitepath = fs::Path(toks.at(0));
  }
  else
  if (opt == "pidfile")
  {
    ParameterCheck(opt, toks, 1);
    pidfile = fs::Path(toks.at(0));
  }
  else if (opt == "port")
  {
    ParameterCheck(opt, toks, 1);
    port = boost::lexical_cast<int>(toks.at(0));
  }
  else if (opt == "default_flags")
  {
    defaultFlags = toks.at(0);
  }
  else if (opt == "tls_certificate")
  {
    ParameterCheck(opt, toks, 1);
    tlsCertificate = fs::Path(toks.at(0));
  }
  else if (opt == "reload_3")
  {
    NotImplemented(opt);
  }
  else if (opt == "datapath")
  {
    ParameterCheck(opt, toks, 1);
    datapath = fs::Path(toks.at(0));
  }
  else if (opt == "pwd_path")
  {
    NotImplemented(opt);
  }
  else if (opt == "grp_path")
  {
    NotImplemented(opt);
  }
  else if (opt == "banner")
  {
    ParameterCheck(opt, toks, 1);
    banner = fs::Path(toks.at(0));
  }
  else if (opt == "ascii_downloads")
  {
    asciiDownloads = setting::AsciiDownloads(toks);
  }
  else if (opt == "ascii_uploads")
  {
    asciiUploads = setting::AsciiUploads(toks);
  }
  else if (opt == "free_space")
  {
    ParameterCheck(opt, toks, 1);
    freeSpace = boost::lexical_cast<int>(toks.at(0));
  }
  else if (opt == "mmap_amount")
  {
    NotImplemented(opt);
  }
  else if (opt == "dl_send_file")
  {
    NotImplemented(opt);
  }
  else if (opt == "ul_buffered_force")
  {
    NotImplemented(opt);
  }
  else if (opt == "total_users")
  {
    ParameterCheck(opt, toks, 1);
    totalUsers = boost::lexical_cast<int>(toks.at(0));
  }
  else if (opt == "multiplier_max")
  {
    ParameterCheck(opt, toks, 1);
    multiplierMax = boost::lexical_cast<int>(toks.at(0));
  }
  else if (opt == "oneliners")
  {
    ParameterCheck(opt, toks, 1);
    oneliners = boost::lexical_cast<int>(toks.at(0));
  }
  else if (opt == "empty_nuke")
  {
    ParameterCheck(opt, toks, 1);
    emptyNuke = boost::lexical_cast<int>(toks.at(0));
  }
  else if (opt == "max_sitecmd_lines")
  {
    ParameterCheck(opt, toks, 1);
    maxSitecmdLines = boost::lexical_cast<int>(toks.at(0));
  }
  else if (opt == "hideuser")
  {
    ParameterCheck(opt, toks, 1, -1);
    hideuser = acl::ACL::FromString(boost::algorithm::join(toks, " "));
  }
  else if (opt == "use_dir_size")
  {
    NotImplemented(opt);
  }
  else if (opt == "timezone")
  {
    ParameterCheck(opt, toks, 1);
    timezone = boost::lexical_cast<int>(toks[0]);
  }
  else if (opt == "dl_incomplete")
  {
    ParameterCheck(opt, toks, 1);
    dlIncomplete = util::string::BoolLexicalCast(toks.at(0));
  }
  else if (opt == "file_dl_count")
  {
    NotImplemented(opt);
  }
  else if (opt == "sitename_long")
  {
    ParameterCheck(opt, toks, 1);
    sitenameLong = toks[0];
  }
  else if (opt == "sitename_short")
  {
    ParameterCheck(opt, toks, 1);
    sitenameShort = toks[0];
  }
  else if (opt == "login_prompt")
  {
    ParameterCheck(opt, toks, 1);
    loginPrompt = toks[0];
  }
  else if (opt == "email")
  {
    ParameterCheck(opt, toks, 1);
    email = toks[0];
  }
  else if (opt == "master")
  {
    ParameterCheck(opt, toks, 1, -1);
    master.insert(master.end(), toks.begin(), toks.end());
  }
  else if (opt == "bouncer_ip")
  {
    ParameterCheck(opt, toks, 1, -1);
    bouncerIp.insert(bouncerIp.end(), toks.begin(), toks.end());
  }
  else if (opt == "calc_crc")
  {
    ParameterCheck(opt, toks, 1, -1);
    calcCrc.insert(calcCrc.end(), toks.begin(), toks.end());
  }
  else if (opt == "xdupe")
  {
    ParameterCheck(opt, toks, 1, -1);
    xdupe.insert(xdupe.end(), toks.begin(), toks.end());
  }
  else if (opt == "valid_ip")
  {
    ParameterCheck(opt, toks, 1, -1);
    validIp.insert(validIp.end(), toks.begin(), toks.end());
  }
  else if (opt == "active_addr")
  {
    ParameterCheck(opt, toks, 1, -1);
    activeAddr.insert(activeAddr.end(), toks.begin(), toks.end());
  }
  else if (opt == "ignore_type")
  { 
    ParameterCheck(opt, toks, 1, -1);
    ignoreType.insert(ignoreType.end(), toks.begin(), toks.end());
  }
  else if (opt == "banned_users")
  {
    ParameterCheck(opt, toks, 1, -1);
    bannedUsers.insert(bannedUsers.end(), toks.begin(), toks.end());
  }
  else if (opt == "idle_commands")
  {
    ParameterCheck(opt, toks, 1, -1);
    idleCommands.insert(idleCommands.end(), toks.begin(), toks.end());
    for (auto& cmd : idleCommands) boost::to_upper(cmd);
  }
  else if (opt == "noretrieve")
  {
    ParameterCheck(opt, toks, 1, -1);
    noretrieve.insert(noretrieve.end(), toks.begin(), toks.end());
  }
  else if (opt == "tagline")
  {
    ParameterCheck(opt, toks, 1);
    tagline = toks[0];
  }
  else if (opt == "speed_limit")
  {
    ParameterCheck(opt, toks, 4, -1);
    speedLimit.emplace_back(toks);
  }
  else if (opt == "sim_xfers")
  {
    ParameterCheck(opt, toks, 2);
    simXfers.emplace_back(toks);
  }
  else if (opt == "secure_ip")
  {
    ParameterCheck(opt, toks, 4, -1);
    secureIp.emplace_back(toks);
  }
  else if (opt == "secure_pass")
  {
    ParameterCheck(opt, toks, 2, -1);
    securePass.emplace_back(toks);
  }
  else if (opt == "pasv_addr")
  {
    ParameterCheck(opt, toks, 1);
    pasvAddr.emplace_back(toks.at(0));
  }
  else if (opt == "active_ports")
  {
    ParameterCheck(opt, toks, 1, -1);
    activePorts = setting::Ports(toks);
  }
  else if (opt == "pasv_ports")
  {
    pasvPorts = setting::Ports(toks);
  }
  else if (opt == "allow_fxp")
  {
    ParameterCheck(opt, toks, 4, -1);
    allowFxp.emplace_back(toks);
  }
  else if (opt == "welcome_msg")
  {
    ParameterCheck(opt, toks, 2, -1);
    welcomeMsg.emplace_back(toks); 
  }
  else if (opt == "goodbye_msg")
  {
    ParameterCheck(opt, toks, 2, -1);
    goodbyeMsg.emplace_back(toks); 
  }
  else if (opt == "newsfile")
  {
    ParameterCheck(opt, toks, 2, -1);
    newsfile.emplace_back(toks); 
  }
  else if (opt == "cdpath")
  {
    ParameterCheck(opt, toks, 1);
    cdpath.emplace_back(toks[0]);
  }
  else if (opt == "nodupecheck")
  {
    ParameterCheck(opt, toks, 1);
    nodupecheck = fs::Path(toks.at(0));
  }
  else if (opt == "alias")
  {
    ParameterCheck(opt, toks, 2);
    alias.emplace_back(toks); 
  }
  else if (opt == "delete")
  {
    ParameterCheck(opt, toks, 2, -1);
    delete_.emplace_back(toks);
  }
  else if (opt == "deleteown")
  {
    ParameterCheck(opt, toks, 2, -1);
    deleteown.emplace_back(toks);
  }
  else if (opt == "overwrite")
  {
    ParameterCheck(opt, toks, 2, -1);
    overwrite.emplace_back(toks);
  }
  else if (opt == "resume")
  {
    ParameterCheck(opt, toks, 2, -1);
    resume.emplace_back(toks);
  }
  else if (opt == "rename")
  {
    ParameterCheck(opt, toks, 2, -1);
    rename.emplace_back(toks);
  }
  else if (opt == "renameown")
  {
    ParameterCheck(opt, toks, 2, -1);
    renameown.emplace_back(toks);
  }
  else if (opt == "filemove")
  {
    ParameterCheck(opt, toks, 2, -1);
    filemove.emplace_back(toks);
  }
  else if (opt == "makedir")
  {
    ParameterCheck(opt, toks, 2, -1);
    makedir.emplace_back(toks);
  }
  else if (opt == "upload")
  {
    ParameterCheck(opt, toks, 2, -1);
    upload.emplace_back(toks);
  }
  else if (opt == "download")
  {
    ParameterCheck(opt, toks, 2, -1);
    download.emplace_back(toks);
  }
  else if (opt == "nuke")
  {
    ParameterCheck(opt, toks, 2, -1);
    nuke.emplace_back(toks);
  }
  else if (opt == "dirlog")
  {
    ParameterCheck(opt, toks, 2, -1);
    dirlog.emplace_back(toks);
  }
  else if (opt == "hideinwho")
  {
    ParameterCheck(opt, toks, 2, -1);
    hideinwho.emplace_back(toks);
  }
  else if (opt == "freefile")
  {
    ParameterCheck(opt, toks, 2, -1);
    freefile.emplace_back(toks);
  }
  else if (opt == "nostats")
  {
    ParameterCheck(opt, toks, 2, -1);
    nostats.emplace_back(toks);
  }
  else if (opt == "hideowner")
  {
    ParameterCheck(opt, toks, 2, -1);
    hideowner.emplace_back(toks);
  }
  else if (opt == "show_diz")
  {
    ParameterCheck(opt, toks, 2, -1);
    showDiz.emplace_back(toks);
  }
  else if (opt == "stat_section")
  {
    ParameterCheck(opt, toks, 3);
    statSection.emplace_back(toks);
  }
  else if (opt == "path-filter")
  {
    ParameterCheck(opt, toks, 3, -1);
    pathFilter.emplace_back(toks);
  }
  else if (opt == "max_users")
  {
    ParameterCheck(opt, toks, 2);
    maxUsers.emplace_back(toks);
  }
  else if (opt == "max_ustats")
  {
    ParameterCheck(opt, toks, 2, -1);
    maxUstats = setting::ACLInt(toks);
  }
  else if (opt == "max_gstats")
  {
    ParameterCheck(opt, toks, 2, -1);
    maxGstats = setting::ACLInt(toks);
  }
  else if (opt == "show_totals")
  {
    ParameterCheck(opt, toks, 2, -1);
    showTotals.emplace_back(toks); 
  }
  else if (opt == "dupe_check")
  {
    ParameterCheck(opt, toks, 1, 2);
    dupeCheck = setting::DupeCheck(toks);
  }
  else if (opt == "cscript")
  {
    ParameterCheck(opt, toks, 3);
    cscript.emplace_back(toks);
  }
  else if (opt == "lslong")
  {
    ParameterCheck(opt, toks, 2);
    lslong = setting::Lslong(toks);
  }
  else if (opt == "hidden_files")
  {
    ParameterCheck(opt, toks, 1);
    hiddenFiles.emplace_back(toks);
  }
  else if (opt == "creditcheck")
  {
    ParameterCheck(opt, toks, 3, -1);
    creditcheck.emplace_back(toks);
  }
  else if (opt == "creditloss")
  {
    ParameterCheck(opt, toks, 4, -1);
    creditloss.emplace_back(toks);
  }
  else if (opt == "nukedir_style")
  {
    ParameterCheck(opt, toks, 3);
    nukedirStyle = setting::NukedirStyle(toks);
  }
  else if (opt == "privgroup")
  {
    ParameterCheck(opt, toks, 2);
    privgroup.emplace_back(toks);
  }
  else if (opt == "msg_path")
  {
    ParameterCheck(opt, toks, 1, -1);
    msgpath.emplace_back(toks);
  }
  else if (opt == "privpath")
  {
    ParameterCheck(opt, toks, 1, -1);
    privpath.emplace_back(toks); 
  }
  else if (opt == "site_cmd")
  {
    ParameterCheck(opt, toks, 3);
    siteCmd.emplace_back(toks);
  }
  else if (opt == "requests")
  {
    ParameterCheck(opt, toks, 2);
    requests = setting::Requests(toks);
  }
  else if (opt == "idle_timeout")
  {
    ParameterCheck(opt, toks, 3);
    idleTimeout = setting::IdleTimeout(toks);
  }
  else
  {
    throw ConfigError("Invalid config option: " + opt);
  }
}

void Config::NotImplemented(const std::string& opt)
{
  logs::error << "Ignoring not implemented config option: " << opt << logs::endl;
}

void Config::ParameterCheck(const std::string& opt,
                            const std::vector<std::string>& toks, int minimum,
                            int maximum)
{
  int toksSize = static_cast<int>(toks.size());
  if (toksSize < minimum || (maximum != -1 && toksSize > maximum))
    throw ConfigError("Wrong numer of Parameters for " + opt);
}

bool Config::CheckSetting(const std::string& name)
{
  std::unordered_map<std::string, int>::const_iterator it;
  it = settingsCache.find(name);
  return (it != settingsCache.end());
}

void Config::SanityCheck()
{
  if (!CheckSetting("tls_certificate")) throw RequiredSetting("tls_certificate");
  else if (!CheckSetting("sitepath")) throw RequiredSetting("sitepath");
  else if (!CheckSetting("port")) throw RequiredSetting("port");
  else if (!CheckSetting("valid_ip")) throw RequiredSetting("valid_ip");
  
  if (loginPrompt.empty())
    loginPrompt = sitenameLong + ": " + programFullname + " connected.";
}

// end namespace
}

#ifdef CFG_CONFIG_TEST
int main()
{
  try
  {
    cfg::Config config("ftpd.conf");
    logs::debug << "Config loaded." << logs::endl;
    logs::debug << "Download: " << config.Download().size() << logs::endl;
  }
  catch(const cfg::ConfigError& e)
  {
    logs::debug << e.Message() << logs::endl;
    return 1;
  }
  return 0;
}
#endif
  
