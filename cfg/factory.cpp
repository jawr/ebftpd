#include "cfg/factory.hpp"
#include "cfg/setting.hpp"
namespace cfg
{
Factory::Factory()
{
  Register("ascii_downloads", new Creator<setting::AsciiDownloads>);
  Register("shutdown", new Creator<setting::Shutdown>);
  Register("free_space", new Creator<setting::FreeSpace>);
  Register("use_dir_size", new Creator<setting::UseDirSize>);
  Register("timezone", new Creator<setting::Timezone>);
  Register("color_mode", new Creator<setting::ColorMode>);
  Register("sitename_long", new Creator<setting::SitenameLong>);
  Register("sitename_short", new Creator<setting::SitenameShort>);
  Register("login_prompt", new Creator<setting::LoginPrompt>);
  Register("rootpath", new Creator<setting::Rootpath>);
  Register("reload_config", new Creator<setting::ReloadConfig>);
  Register("master", new Creator<setting::Master>);
  Register("secure_ip", new Creator<setting::SecureIp>);
  Register("secure_pass", new Creator<setting::SecurePass>);
  Register("datapath", new Creator<setting::Datapath>);
  Register("pwd_path", new Creator<setting::PwdPath>);
  Register("grp_path", new Creator<setting::GrpPath>);
  Register("botscript_path", new Creator<setting::BotscriptPath>);
  Register("bouncer_ip", new Creator<setting::BouncerIp>);
  Register("speed_limit", new Creator<setting::SpeedLimit>);
  Register("sim_xfers", new Creator<setting::SimXfers>);
  Register("calc_crc", new Creator<setting::CalcCrc>);
  Register("xdupe", new Creator<setting::Xdupe>);
  Register("mmap_amount", new Creator<setting::MmapAmount>);
  Register("dl_sendfile", new Creator<setting::DlSendfile>);
  Register("ul_buffered_force", new Creator<setting::UlBufferedForce>);
  Register("min_homedir", new Creator<setting::MinHomedir>);
  Register("valid_ip", new Creator<setting::ValidIp>);
  Register("active_addr", new Creator<setting::ActiveAddr>);
  Register("pasv_addr", new Creator<setting::PasvAddr>);
  Register("active_ports", new Creator<setting::Ports>);
  Register("pasv_ports", new Creator<setting::Ports>);
  Register("allow_fxp", new Creator<setting::AllowFxp>);
  Register("welcome_msg", new Creator<setting::WelcomeMsg>);
  Register("goodbye_msg", new Creator<setting::GoodbyeMsg>);
  Register("newsfile", new Creator<setting::Newsfile>);
  Register("banner", new Creator<setting::Banner>);
  Register("alias", new Creator<setting::Alias>);
  Register("cdpath", new Creator<setting::Cdpath>);
  Register("ignore_type", new Creator<setting::IgnoreType>);
  // rights
  Register("delete", new Creator<setting::Right>);
  Register("deleteown", new Creator<setting::Right>);
  Register("overwrite", new Creator<setting::Right>);
  Register("resume", new Creator<setting::Right>);
  Register("rename", new Creator<setting::Right>);
  Register("renameown", new Creator<setting::Right>);
  Register("filemove", new Creator<setting::Right>);
  Register("makedir", new Creator<setting::Right>);
  Register("upload", new Creator<setting::Right>);
  Register("download", new Creator<setting::Right>);
  Register("nuke", new Creator<setting::Right>);
  Register("dirlog", new Creator<setting::Right>);
  Register("hideinwho", new Creator<setting::Right>);
  Register("freefile", new Creator<setting::Right>);
  Register("nostats", new Creator<setting::Right>);
  // end rights
  Register("stat_section", new Creator<setting::StatSection>);
  Register("path-filter", new Creator<setting::PathFilter>);
  Register("max_users", new Creator<setting::MaxUsers>);
  Register("max_ustats", new Creator<setting::MaxUstats>);
  Register("max_gstats", new Creator<setting::MaxGstats>);
  Register("banned_users", new Creator<setting::BannedUsers>);
  Register("show_diz", new Creator<setting::ShowDiz>);
  Register("show_totals", new Creator<setting::ShowTotals>);
  Register("dl_incomplete", new Creator<setting::DlIncomplete>);
  Register("file_dl_count", new Creator<setting::FileDlCount>);
  Register("dupe_check", new Creator<setting::DupeCheck>);
  Register("script", new Creator<setting::Script>);
  Register("idle_commands", new Creator<setting::IdleCommands>);
  Register("total_users", new Creator<setting::TotalUsers>);
  Register("lslong", new Creator<setting::Lslong>);
  Register("hidden_files", new Creator<setting::HiddenFiles>);
  Register("noretrieve", new Creator<setting::Noretrieve>);
  Register("tagline", new Creator<setting::Tagline>);
  Register("email", new Creator<setting::Email>);
  Register("multiplier_max", new Creator<setting::MultiplierMax>);
  Register("oneliners", new Creator<setting::Oneliners>);
  Register("requests", new Creator<setting::Requests>);
  Register("lastonline", new Creator<setting::Lastonline>);
  Register("empty_nuke", new Creator<setting::EmptyNuke>);
  Register("nodupecheck", new Creator<setting::Nodupecheck>);
  Register("creditcheck", new Creator<setting::Creditcheck>);
  Register("creditloss", new Creator<setting::Creditloss>);
  Register("nukedir_style", new Creator<setting::NukedirStyle>);
  Register("hideuser", new Creator<setting::Hideuser>);
  Register("privgroup", new Creator<setting::Privgroup>);
  Register("msgpath", new Creator<setting::Msgpath>);
  Register("privpath", new Creator<setting::Privpath>);
  Register("site_cmd", new Creator<setting::SiteCmd>);
  Register("max_sitecmd_lines", new Creator<setting::MaxSitecmdLines>);
  Register("cscript", new Creator<setting::Cscript>);
}

setting::Setting *Factory::Create(const std::string& name)
{
  Registry::const_iterator it = registry.find(name);
  if (it == registry.end()) return 0; 
  return it->second->Create();
}

}
