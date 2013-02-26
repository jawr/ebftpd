#include <iostream>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "cfg/config.hpp"
#include "cfg/error.hpp"
#include "util/string.hpp"
#include "main.hpp"
#include "util/path/path.hpp"
#include "util/format.hpp"

namespace util
{

template <> const char* util::EnumStrings<cfg::EPSVFxp>::values[] = 
{
  "Allow",
  "Deny",
  "Force",
  ""
};

}

namespace cfg
{

int Config::latestVersion = 0;
std::unordered_set<std::string> Config::aclKeywords;
const std::vector<std::string> Config::requiredSettings
{
  "sitepath",
  "datapath",
  "port",
  "valid_ip"
};

const std::string Config::configFile = "ebftpd.conf";
const std::vector<std::string> Config::configSearch = { "../etc", "etc", "." };
std::string Config::lastConfigPath;

Config::Config(const std::string& configPath, bool tool) : 
  version(++latestVersion),
  tool(tool),
  currentSection(nullptr),
  port(-1),
  freeSpace(100),
  timezone(0),
  sitenameLong("SITE NAME"),
  sitenameShort("SN"),
  datapath("data"),
  bouncerOnly(false),
  securityLog("security", true, true, 0),
  databaseLog("database", true, true, 0),
  eventLog("events", true, true, 0),
  errorLog("errors", true, true, 0),
  debugLog("debug", true, true, 0),
  siteopLog("siteop", true, true, 0),
  dlIncomplete(true),
  totalUsers(20),
  multiplierMax(10),
  emptyNuke(102400),
  maxSitecmdLines(-1),
  weekStart(::cfg::WeekStart::Sunday),
  epsvFxp(::cfg::EPSVFxp::Allow),
  maximumRatio(10),
  dirSizeDepth(2),
  asyncCRC(false),
  tlsControl("*"),
  tlsListing("*"),
  tlsData("!*"),
  tlsFxp("!*")
{
  std::string line;
  std::ifstream io(configPath.c_str());
  if (!io) throw ConfigError("Unable to open config file.");

  for (int i = 1; std::getline(io, line); ++i)
  {
    std::string::size_type pos = line.find_first_of('#');
    if (pos != std::string::npos) line.erase(pos);
    if (line.empty()) continue;
    
    try 
    {
      Parse(line);
    } 
    catch (const ConfigError& e)
    {
      throw ConfigError(util::Format()("Error in config at line %1%: %2%", i, e.Message()));
    }
    catch (const std::bad_cast& e)
    {
      throw ConfigError(util::Format()("Error in config at line %1%: Invalid value", i));
    }
  }

  SanityCheck();
}

void Config::ParseGlobal(const std::string& opt, std::vector<std::string>& toks)
{
  if (opt[0] == '-')
  {
    ParameterCheck(opt, toks, 1, -1);
    std::string keyword(opt.substr(1));
    if (aclKeywords.find(keyword) == aclKeywords.end() && !tool)
      throw ConfigError("Invalid command acl keyword: " + keyword);
    commandACLs.insert(std::make_pair(keyword, 
        acl::ACL(util::Join(toks, " "))));
  }
  else
  if (util::StartsWith(opt, "custom-"))
  {
    ParameterCheck(opt, toks, 1, -1);
    std::string command(util::ToUpperCopy(opt.substr(7)));
    if (std::find_if(siteCmd.begin(), siteCmd.end(), 
        [&](const setting::SiteCmd& sc)
        { return command == sc.Command();}) == siteCmd.end() && !tool)
    {
      throw ConfigError("Invalid custom command acl keyword: " + command);
    }
    commandACLs.insert(std::make_pair(util::ToLowerCopy(opt), 
        acl::ACL(util::Join(toks, " "))));
  }
  else
  if (opt == "database")
  {
    ParameterCheck(opt, toks, 3, 5);
    database = setting::Database(toks);
  }
  else
  if (opt == "sitepath")
  {
    ParameterCheck(opt, toks, 1);
    sitepath = toks[0];
  }
  else
  if (opt == "pidfile")
  {
    ParameterCheck(opt, toks, 1);
    pidfile = toks[0];
  }
  else if (opt == "port")
  {
    ParameterCheck(opt, toks, 1);
    port = boost::lexical_cast<int>(toks[0]);
  }
  else if (opt == "tls_certificate")
  {
    ParameterCheck(opt, toks, 1);
    tlsCertificate = toks[0];
  }
  else if (opt == "tls_ciphers")
  {
    ParameterCheck(opt, toks, 1);
    tlsCiphers = toks[0];
  }
  else if (opt == "datapath")
  {
    ParameterCheck(opt, toks, 1);
    datapath = toks[0];
  }
  else if (opt == "banner")
  {
    ParameterCheck(opt, toks, 1);
    banner = toks[0];
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
    freeSpace = boost::lexical_cast<int>(toks[0]);
  }
  else if (opt == "total_users")
  {
    ParameterCheck(opt, toks, 1);
    totalUsers = boost::lexical_cast<int>(toks[0]);
  }
  else if (opt == "multiplier_max")
  {
    ParameterCheck(opt, toks, 1);
    multiplierMax = boost::lexical_cast<int>(toks[0]);
  }
  else if (opt == "empty_nuke")
  {
    ParameterCheck(opt, toks, 1);
    emptyNuke = boost::lexical_cast<int>(toks[0]);
  }
  else if (opt == "max_sitecmd_lines")
  {
    ParameterCheck(opt, toks, 1);
    maxSitecmdLines = boost::lexical_cast<int>(toks[0]);
  }
  else if (opt == "timezone")
  {
    ParameterCheck(opt, toks, 1);
    timezone = boost::lexical_cast<int>(toks[0]);
  }
  else if (opt == "dl_incomplete")
  {
    ParameterCheck(opt, toks, 1);
    dlIncomplete = util::BoolLexicalCast(toks[0]);
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
  else if (opt == "security_log")
  {
    ParameterCheck(opt, toks, 3, 3);
    securityLog = setting::Log("security", toks);
  }
  else if (opt == "database_log")
  {
    ParameterCheck(opt, toks, 2, 2);
    databaseLog = setting::Log("database", toks);
  }
  else if (opt == "event_log")
  {
    ParameterCheck(opt, toks, 3, 3);
    eventLog = setting::Log("events", toks);
  }
  else if (opt == "debug_log")
  {
    ParameterCheck(opt, toks, 3, 3);
    debugLog = setting::Log("debug", toks);
  }
  else if (opt == "error_log")
  {
    ParameterCheck(opt, toks, 3, 3);
    errorLog = setting::Log("errors", toks);
  }
  else if (opt == "siteop_log")
  {
    ParameterCheck(opt, toks, 3, 3);
    siteopLog = setting::Log("siteop", toks);
  }
  else if (opt == "bouncer_only")
  {
    ParameterCheck(opt, toks, 1);
    bouncerOnly = util::BoolLexicalCast(toks[0]);
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
  else if (opt == "banned_users")
  {
    ParameterCheck(opt, toks, 1, -1);
    bannedUsers.insert(bannedUsers.end(), toks.begin(), toks.end());
  }
  else if (opt == "idle_commands")
  {
    ParameterCheck(opt, toks, 1, -1);
    idleCommands.insert(idleCommands.end(), toks.begin(), toks.end());
    for (auto& cmd : idleCommands) util::ToUpper(cmd);
  }
  else if (opt == "noretrieve")
  {
    ParameterCheck(opt, toks, 1, -1);
    noretrieve.insert(noretrieve.end(), toks.begin(), toks.end());
  }
  else if (opt == "maximum_speed")
  {
    ParameterCheck(opt, toks, 3, -1);
    maximumSpeed.emplace_back(toks);
  }
  else if (opt == "minimum_speed")
  {
    ParameterCheck(opt, toks, 3, -1);
    minimumSpeed.emplace_back(toks);
  }
  else if (opt == "sim_xfers")
  {
    ParameterCheck(opt, toks, 2);
    simXfers = setting::SimXfers(toks);
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
    pasvAddr.emplace_back(toks[0]);
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
    ParameterCheck(opt, toks, 3, -1);
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
  else if (opt == "cdpath")
  {
    ParameterCheck(opt, toks, 1);
    cdpath.emplace_back(toks[0]);
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
  else if (opt == "event_path")
  {
    ParameterCheck(opt, toks, 1);
    eventpath.emplace_back(toks[0]);
  }
  else if (opt == "dupe_path")
  {
    ParameterCheck(opt, toks, 1);
    dupepath.emplace_back(toks[0]);
  }
  else if (opt == "index_path")
  {
    ParameterCheck(opt, toks, 1);
    indexpath.emplace_back(toks[0]);
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
  else if (opt == "path-filter")
  {
    ParameterCheck(opt, toks, 3, -1);
    pathFilter.emplace_back(toks);
  }
  else if (opt == "max_users")
  {
    ParameterCheck(opt, toks, 2);
    maxUsers = setting::MaxUsers(toks);
  }
  else if (opt == "max_ustats")
  {
    ParameterCheck(opt, toks, 1, -1);
    maxUstats.emplace_back(toks);
  }
  else if (opt == "max_gstats")
  {
    ParameterCheck(opt, toks, 1, -1);
    maxGstats.emplace_back(toks);
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
    ParameterCheck(opt, toks, 2, -1);
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
    ParameterCheck(opt, toks, 4, 5);
    siteCmd.emplace_back(toks);
  }
  else if (opt == "idle_timeout")
  {
    ParameterCheck(opt, toks, 3);
    idleTimeout = setting::IdleTimeout(toks);
  }
  else if (opt == "week_start")
  {
    ParameterCheck(opt, toks, 1);
    util::ToLower(toks[0]);
    if (toks[0] == "sunday") weekStart = ::cfg::WeekStart::Sunday;
    else if (toks[0] == "monday") weekStart = ::cfg::WeekStart::Monday;
    else throw ConfigError("week_start must be either sunday or monday.");
  }
  else if (opt == "pre_check")
  {
    ParameterCheck(opt, toks, 1, 2);
    preCheck.emplace_back(toks);
  }
  else if (opt == "pre_dir_check")
  {
    ParameterCheck(opt, toks, 1, 2);
    preDirCheck.emplace_back(toks);
  }
  else if (opt == "post_check")
  {
    ParameterCheck(opt, toks, 1, 2);
    postCheck.emplace_back(toks);
  }
  else if (opt == "section")
  {
    ParameterCheck(opt, toks, 1);
    util::ToUpper(toks[0]);
    auto result = sections.insert(std::make_pair(toks[0], Section(toks[0])));
    if (!result.second) throw ConfigError("Section " + toks[0] + " already exists.");
    currentSection = &result.first->second;
  }
  else if (opt == "epsv_fxp")
  {
    ParameterCheck(opt, toks, 1);
    util::ToLower(toks[0]);
    if (toks[0] == "allow") epsvFxp = ::cfg::EPSVFxp::Allow;
    else if (toks[0] == "deny") epsvFxp = ::cfg::EPSVFxp::Deny;
    else if (toks[0] == "force") epsvFxp = ::cfg::EPSVFxp::Force;
    else throw ConfigError("epsv_fxp must be allow, deny or force");
  }
  else if (opt == "maximum_ratio")
  {
    ParameterCheck(opt, toks, 1);
    try
    {
      maximumRatio = boost::lexical_cast<int>(toks[0]);
      if (maximumRatio < 0) throw boost::bad_lexical_cast();
    }
    catch (const boost::bad_lexical_cast&)
    {
      throw ConfigError("maximum_ratio must be zero or larger");
    }
  }
  else if (opt == "dir_size_depth")
  {
    ParameterCheck(opt, toks, 1);
    try
    {
      dirSizeDepth = boost::lexical_cast<int>(toks[0]);
      if (dirSizeDepth < 0) throw boost::bad_lexical_cast();
    }
    catch (const boost::bad_lexical_cast&)
    {
      throw ConfigError("dir_size_depth must be zero or larger");
    }
  }
  else if (opt == "async_crc")
  {
    ParameterCheck(opt, toks, 1);
    asyncCRC = util::BoolLexicalCast(toks[0]);
  }
  else if (opt == "tls_control")
  {
    tlsControl = acl::ACL(util::Join(toks, " "));
  }
  else if (opt == "tls_listing")
  {
    tlsListing = acl::ACL(util::Join(toks, " "));
  }
  else if (opt == "tls_data")
  {
    tlsData = acl::ACL(util::Join(toks, " "));
  }
  else if (opt == "tls_fxp")
  {
    tlsFxp = acl::ACL(util::Join(toks, " "));
  }
  else
  {
    throw ConfigError("Invalid global config option: " + opt);
  }
}

void Config::ParseSection(const std::string& opt, std::vector<std::string>& toks)
{
  if (opt == "path")
  {
    ParameterCheck(opt, toks, 1);
    currentSection->paths.emplace_back(toks[0]);
  }
  else if (opt == "separate_credits")
  {
    ParameterCheck(opt, toks, 1);
    currentSection->separateCredits = util::BoolLexicalCast(toks[0]);
  }
  else if (opt == "ratio")
  {
    ParameterCheck(opt, toks, 1);
    try
    {
      currentSection->ratio = boost::lexical_cast<int>(toks[0]);
      if (currentSection->ratio < 0) throw boost::bad_lexical_cast();
    }
    catch (const boost::bad_lexical_cast&)
    {
      throw ConfigError("ratio must be zero or larger");
    }
  }
  else if (opt == "endsection")
  {
    currentSection = nullptr;
  }
  else
  {
    throw ConfigError("Invalid section config option: " + opt);
  }
}

void Config::Parse(std::string line)
{
  std::vector<std::string> toks;
  
  util::Trim(line);
  util::Split(toks, line, "\t ", true);
  for (auto& token : toks) boost::replace_all(token, "[:space:]", " ");

  if (toks.size() == 0) return;
  std::string opt = toks[0];
  if (opt.size() == 0) return;
  
  // remove setting from args
  toks.erase(toks.begin());    
  util::ToLower(opt);

  // update cache for sanity check
  settingsCache[opt]++; 

  if (currentSection) ParseSection(opt, toks);
  else ParseGlobal(opt, toks);  
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
  return settingsCache.find(name) != settingsCache.end();
}

void Config::SanityCheck()
{
  for (const std::string& setting : requiredSettings)
  {
    if (!CheckSetting(setting)) throw RequiredSettingError(setting);
  }
  
  if (loginPrompt.empty())
    loginPrompt = sitenameLong + ": ebftpd connected.";
    
  if (allowFxp.empty()) allowFxp.emplace_back();
}

bool Config::IsBouncer(const std::string& ip) const
{
  for (const std::string& bip : bouncerIp)
    if (ip == bip) return true;
  return false;

}

boost::optional<const Section&> Config::SectionMatch(const std::string& path) const
{
  for (const auto& kv : sections)
  {
    if (kv.second.IsMatch(path)) 
      return boost::optional<const Section&>(kv.second);
  }
      
  return boost::optional<const Section&>();
}

ConfigPtr Config::Load(std::string configPath, bool tool)
{
  std::vector<std::string> configPaths;
  if (!configPath.empty())
  {
    configPaths.push_back(configPath);
  }
  else
  {
    for (const std::string& cs : configSearch)
    {
      configPaths.push_back(util::path::Join(cs, configFile));
    }
  }
  
  bool exists = false;
  for (const std::string& cp : configPaths)
  {
    if (!access(cp.c_str(), F_OK))
    {
      lastConfigPath = cp;
      exists = true;
      break;
    }
  }
  
  if (!exists) throw ConfigError("Unable to open config file.");  
  return std::make_shared<Config>(lastConfigPath, tool);
}

bool Config::IsEventLogged(const std::string& path) const
{
  if (path.empty()) return false;
  return util::WildcardMatch(eventpath, path + (path.back() != '/' ? "/" : ""));
}

bool Config::IsDupeLogged(const std::string& path) const
{
  return util::WildcardMatch(dupepath, path + (path.back() != '/' ? "/" : ""));
}

bool Config::IsIndexed(const std::string& path) const
{
  return util::WildcardMatch(indexpath, path + (path.back() != '/' ? "/" : ""));
}

// end namespace
}
