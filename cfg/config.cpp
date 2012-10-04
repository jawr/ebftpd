#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/function.hpp>                                                   
#include <boost/bind.hpp>                                                       
#include "cfg/config.hpp"
#include "cfg/exception.hpp"
#include "cfg/setting.hpp"
#include "util/string.hpp"
#include "logger/logger.hpp"

namespace cfg
{

int Config::latestVersion = 0;

Config::Config(const std::string& config) : version(++latestVersion), config(config)
{
  // register
  registry.insert(std::make_pair("ascii_downloads", boost::bind(&Config::AddAsciiDownloads, this, _1)));
  registry.insert(std::make_pair("shutdown", boost::bind(&Config::AddShutdown, this, _1)));
  registry.insert(std::make_pair("free_space", boost::bind(&Config::AddFreeSpace, this, _1)));
  registry.insert(std::make_pair("use_dir_size", boost::bind(&Config::AddUseDirSize, this, _1)));
  registry.insert(std::make_pair("timezone", boost::bind(&Config::AddTimezone, this, _1)));
  registry.insert(std::make_pair("color_mode", boost::bind(&Config::AddColorMode, this, _1)));
  registry.insert(std::make_pair("sitename_long", boost::bind(&Config::AddSitenameLong, this, _1)));
  registry.insert(std::make_pair("sitename_short", boost::bind(&Config::AddSitenameShort, this, _1)));
  registry.insert(std::make_pair("login_prompt", boost::bind(&Config::AddLoginPrompt, this, _1)));
  registry.insert(std::make_pair("rootpath", boost::bind(&Config::AddRootpath, this, _1)));
  registry.insert(std::make_pair("reload_config", boost::bind(&Config::AddReloadConfig, this, _1)));
  registry.insert(std::make_pair("master", boost::bind(&Config::AddMaster, this, _1)));
  registry.insert(std::make_pair("secure_ip", boost::bind(&Config::AddSecureIp, this, _1)));
  registry.insert(std::make_pair("secure_pass", boost::bind(&Config::AddSecurePass, this, _1)));
  registry.insert(std::make_pair("datapath", boost::bind(&Config::AddDatapath, this, _1)));
  registry.insert(std::make_pair("pwd_path", boost::bind(&Config::AddPwdPath, this, _1)));
  registry.insert(std::make_pair("grp_path", boost::bind(&Config::AddGrpPath, this, _1)));
  registry.insert(std::make_pair("botscript_path", boost::bind(&Config::AddBotscriptPath, this, _1)));
  registry.insert(std::make_pair("bouncer_ip", boost::bind(&Config::AddBouncerIp, this, _1)));
  registry.insert(std::make_pair("speed_limit", boost::bind(&Config::AddSpeedLimit, this, _1)));
  registry.insert(std::make_pair("sim_xfers", boost::bind(&Config::AddSimXfers, this, _1)));
  registry.insert(std::make_pair("calc_crc", boost::bind(&Config::AddCalcCrc, this, _1)));
  registry.insert(std::make_pair("xdupe", boost::bind(&Config::AddXdupe, this, _1)));
  registry.insert(std::make_pair("mmap_amount", boost::bind(&Config::AddMmapAmount, this, _1)));
  registry.insert(std::make_pair("dl_sendfile", boost::bind(&Config::AddDlSendfile, this, _1)));
  registry.insert(std::make_pair("ul_buffered_force", boost::bind(&Config::AddUlBufferedForce, this, _1)));
  registry.insert(std::make_pair("min_homedir", boost::bind(&Config::AddMinHomedir, this, _1)));
  registry.insert(std::make_pair("valid_ip", boost::bind(&Config::AddValidIp, this, _1)));
  registry.insert(std::make_pair("active_addr", boost::bind(&Config::AddActiveAddr, this, _1)));
  registry.insert(std::make_pair("pasv_addr", boost::bind(&Config::AddPasvAddr, this, _1)));
  registry.insert(std::make_pair("active_ports", boost::bind(&Config::AddActivePorts, this, _1)));
  registry.insert(std::make_pair("pasv_ports", boost::bind(&Config::AddPasvPorts, this, _1)));
  registry.insert(std::make_pair("allow_fxp", boost::bind(&Config::AddAllowFxp, this, _1)));
  registry.insert(std::make_pair("welcome_msg", boost::bind(&Config::AddWelcomeMsg, this, _1)));
  registry.insert(std::make_pair("goodbye_msg", boost::bind(&Config::AddGoodbyeMsg, this, _1)));
  registry.insert(std::make_pair("newsfile", boost::bind(&Config::AddNewsfile, this, _1)));
  registry.insert(std::make_pair("banner", boost::bind(&Config::AddBanner, this, _1)));
  registry.insert(std::make_pair("alias", boost::bind(&Config::AddAlias, this, _1)));
  registry.insert(std::make_pair("cdpath", boost::bind(&Config::AddCdpath, this, _1)));
  registry.insert(std::make_pair("ignore_type", boost::bind(&Config::AddIgnoreType, this, _1)));
  registry.insert(std::make_pair("delete", boost::bind(&Config::AddDelete, this, _1)));
  registry.insert(std::make_pair("deleteown", boost::bind(&Config::AddDeleteown, this, _1)));
  registry.insert(std::make_pair("overwrite", boost::bind(&Config::AddOverwrite, this, _1)));
  registry.insert(std::make_pair("resume", boost::bind(&Config::AddResume, this, _1)));
  registry.insert(std::make_pair("rename", boost::bind(&Config::AddRename, this, _1)));
  registry.insert(std::make_pair("renameown", boost::bind(&Config::AddRenameown, this, _1)));
  registry.insert(std::make_pair("filemove", boost::bind(&Config::AddFilemove, this, _1)));
  registry.insert(std::make_pair("makedir", boost::bind(&Config::AddMakedir, this, _1)));
  registry.insert(std::make_pair("upload", boost::bind(&Config::AddUpload, this, _1)));
  registry.insert(std::make_pair("download", boost::bind(&Config::AddDownload, this, _1)));
  registry.insert(std::make_pair("nuke", boost::bind(&Config::AddNuke, this, _1)));
  registry.insert(std::make_pair("dirlog", boost::bind(&Config::AddDirlog, this, _1)));
  registry.insert(std::make_pair("hideinwho", boost::bind(&Config::AddHideinwho, this, _1)));
  registry.insert(std::make_pair("freefile", boost::bind(&Config::AddFreefile, this, _1)));
  registry.insert(std::make_pair("nostats", boost::bind(&Config::AddNostats, this, _1)));
  registry.insert(std::make_pair("stat_section", boost::bind(&Config::AddStatSection, this, _1)));
  registry.insert(std::make_pair("path-filter", boost::bind(&Config::AddPathFilter, this, _1)));
  registry.insert(std::make_pair("max_users", boost::bind(&Config::AddMaxUsers, this, _1)));
  registry.insert(std::make_pair("max_ustats", boost::bind(&Config::AddMaxUstats, this, _1)));
  registry.insert(std::make_pair("max_gstats", boost::bind(&Config::AddMaxGstats, this, _1)));
  registry.insert(std::make_pair("banned_users", boost::bind(&Config::AddBannedUsers, this, _1)));
  registry.insert(std::make_pair("show_diz", boost::bind(&Config::AddShowDiz, this, _1)));
  registry.insert(std::make_pair("show_totals", boost::bind(&Config::AddShowTotals, this, _1)));
  registry.insert(std::make_pair("dl_incomplete", boost::bind(&Config::AddDlIncomplete, this, _1)));
  registry.insert(std::make_pair("file_dl_count", boost::bind(&Config::AddFileDlCount, this, _1)));
  registry.insert(std::make_pair("dupe_check", boost::bind(&Config::AddDupeCheck, this, _1)));
  registry.insert(std::make_pair("script", boost::bind(&Config::AddScript, this, _1)));
  registry.insert(std::make_pair("idle_commands", boost::bind(&Config::AddIdleCommands, this, _1)));
  registry.insert(std::make_pair("total_users", boost::bind(&Config::AddTotalUsers, this, _1)));
  registry.insert(std::make_pair("lslong", boost::bind(&Config::AddLslong, this, _1)));
  registry.insert(std::make_pair("hidden_files", boost::bind(&Config::AddHiddenFiles, this, _1)));
  registry.insert(std::make_pair("noretrieve", boost::bind(&Config::AddNoretrieve, this, _1)));
  registry.insert(std::make_pair("tagline", boost::bind(&Config::AddTagline, this, _1)));
  registry.insert(std::make_pair("email", boost::bind(&Config::AddEmail, this, _1)));
  registry.insert(std::make_pair("multiplier_max", boost::bind(&Config::AddMultiplierMax, this, _1)));
  registry.insert(std::make_pair("oneliners", boost::bind(&Config::AddOneliners, this, _1)));
  registry.insert(std::make_pair("requests", boost::bind(&Config::AddRequests, this, _1)));
  registry.insert(std::make_pair("lastonline", boost::bind(&Config::AddLastonline, this, _1)));
  registry.insert(std::make_pair("empty_nuke", boost::bind(&Config::AddEmptyNuke, this, _1)));
  registry.insert(std::make_pair("nodupecheck", boost::bind(&Config::AddNodupecheck, this, _1)));
  registry.insert(std::make_pair("creditcheck", boost::bind(&Config::AddCreditcheck, this, _1)));
  registry.insert(std::make_pair("creditloss", boost::bind(&Config::AddCreditloss, this, _1)));
  registry.insert(std::make_pair("nukedir_style", boost::bind(&Config::AddNukedirStyle, this, _1)));
  registry.insert(std::make_pair("hideuser", boost::bind(&Config::AddHideuser, this, _1)));
  registry.insert(std::make_pair("privgroup", boost::bind(&Config::AddPrivgroup, this, _1)));
  registry.insert(std::make_pair("msgpath", boost::bind(&Config::AddMsgpath, this, _1)));
  registry.insert(std::make_pair("privpath", boost::bind(&Config::AddPrivpath, this, _1)));
  registry.insert(std::make_pair("site_cmd", boost::bind(&Config::AddSiteCmd, this, _1)));
  registry.insert(std::make_pair("max_sitecmd_lines", boost::bind(&Config::AddMaxSitecmdLines, this, _1)));
  registry.insert(std::make_pair("cscript", boost::bind(&Config::AddCscript, this, _1)));


  // end register

  std::string line;
  std::ifstream io(config.c_str(), std::ifstream::in);
  int i = 0;

  if (!io.is_open()) throw ConfigFileError();

  Factory f;

  while (io.good())
  {
    std::getline(io, line);
    ++i;
    if (line.size() == 0) continue;
    if (line.size() > 0 && line.at(0) == '#') continue;
    try 
    {
      Parse(line, f);
    } 
    catch (NoSetting &e) // handle properly
    {
      ::logger::ftpd << e.what() << " (" << config << ":" << i << ")" << logger::endl;
    }
    catch (...)
    {
      logger::ftpd << "super error on line " << i << logger::endl;
      logger::ftpd << line << logger::endl;
      throw;
    }
  }
}

void Config::Parse(const std::string& line, Factory& factory) {
  std::vector<std::string> toks;
  boost::split(toks, line, boost::is_any_of("\t "));
  if (toks.size() == 0) return;
  std::string opt = toks.at(0);
  if (opt.size() == 0) return;
  // remove setting from args
  toks.erase(toks.begin());
  std::vector<std::string>::iterator it;
  for (it = toks.begin(); it != toks.end();)
    if ((*it).size() == 0)
      it = toks.erase(it);
    else
      ++it;
    

  // parse string
  boost::algorithm::to_lower(opt);

  // plan to rehaul this area in the future to sway from glftpd's inconsitencies
  // check if we have a perm to parse
  if (opt.at(0) == '-' || opt.find("custom-") != std::string::npos)
  {
    std::vector<std::string> temp;
    boost::split(temp, opt, boost::is_any_of("-"));
    if (temp.size() > 1) temp.erase(temp.begin());
    // loop and update
    return;
  }

  cfg::setting::Setting *set = factory.Create(opt);
  if (set == 0)
    throw cfg::NoSetting("No setting found for: " + opt);
  set->Save(toks);

  
  registry[opt](set);

  // todo: build internal cache to easily manage required
  
}


}

#ifdef CFG_CONFIG_TEST
int main()
{
  cfg::Config c("glftpd.conf");
  const std::vector<cfg::setting::Right*>& x = c.Download();
  logger::ftpd << "Download: " << x.size() << logger::endl;
  return 0;
}
#endif
  
