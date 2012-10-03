#include "cfg/factory.hpp"
#include "cfg/setting.hpp"
namespace cfg
{
Factory::Factory()
{
  Register("ascii_downloads", new Creator<AsciiDownloads>);
  Register("shutdown", new Creator<Shutdown>);
  Register("free_space", new Creator<FreeSpace>);
  Register("use_dir_size", new Creator<UseDirSize>);
  Register("timezone", new Creator<Timezone>);
  Register("color_mode", new Creator<ColorMode>);
  Register("sitename_long", new Creator<SitenameLong>);
  Register("sitename_short", new Creator<SitenameShort>);
  Register("login_prompt", new Creator<LoginPrompt>);
  Register("root_path", new Creator<RootPath>);
  Register("reload_config", new Creator<ReloadConfig>);
  Register("master", new Creator<Master>);
  Register("secure_ip", new Creator<SecureIp>);
  Register("secure_pass", new Creator<SecurePass>);
  Register("data_path", new Creator<DataPath>);
  Register("pwd_path", new Creator<PwdPath>);
  Register("grp_path", new Creator<GrpPath>);
  Register("botscript_path", new Creator<BotscriptPath>);
  Register("bouncer_ip", new Creator<BouncerIp>);
  Register("speed_limit", new Creator<SpeedLimit>);
  Register("sim_xfers", new Creator<SimXfers>);
  Register("calc_crc", new Creator<CalcCrc>);
  Register("xdupe", new Creator<Xdupe>);
  Register("mmap_amount", new Creator<MmapAmount>);
  Register("dl_sendfile", new Creator<DlSendfile>);
  Register("ul_buffered_force", new Creator<UlBufferedForce>);
  Register("min_homedir", new Creator<MinHomedir>);
  Register("valid_ip", new Creator<ValidIp>);
  Register("active_addr", new Creator<ActiveAddr>);
  Register("pasv_addr", new Creator<PasvAddr>);
  Register("active_ports", new Creator<ActivePorts>);
  Register("pasv_ports", new Creator<PasvPorts>);
  Register("allow_fxp", new Creator<AllowFxp>);
  Register("welcome_msg", new Creator<WelcomeMsg>);
  Register("goodbye_msg", new Creator<GoodbyeMsg>);
  Register("newsfile", new Creator<Newsfile>);
  Register("banner", new Creator<Banner>);
  Register("alias", new Creator<Alias>);
  Register("cdpath", new Creator<Cdpath>);
  Register("ignore_type", new Creator<IgnoreType>);
  Register("delete", new Creator<Delete>);
  Register("deleteown", new Creator<Deleteown>);
  Register("overwrite", new Creator<Overwrite>);
  Register("resume", new Creator<Resume>);
  Register("rename", new Creator<Rename>);
  Register("renameown", new Creator<Renameown>);
  Register("filemove", new Creator<Filemove>);
  Register("makedir", new Creator<Makedir>);
  Register("upload", new Creator<Upload>);
  Register("download", new Creator<Download>);
  Register("nuke", new Creator<Nuke>);
  Register("dirlog", new Creator<Dirlog>);
  Register("hideinwho", new Creator<Hideinwho>);
  Register("freefile", new Creator<Freefile>);
  Register("stat_section", new Creator<StatSection>);
  Register("path-filter", new Creator<PathFilter>);
  Register("max_users", new Creator<MaxUsers>);
  Register("max_ustats", new Creator<MaxUstats>);
  Register("max_gstats", new Creator<MaxGstats>);
  Register("banned_users", new Creator<BannedUsers>);
  Register("show_diz", new Creator<ShowDiz>);
  Register("show_totals", new Creator<ShowTotals>);
  Register("dl_incomplete", new Creator<DlIncomplete>);
  Register("file_dl_count", new Creator<FileDlCount>);
  Register("dupe_check", new Creator<DupeCheck>);
  Register("script", new Creator<Script>);
  Register("idle_commands", new Creator<IdleCommands>);
  Register("total_users", new Creator<TotalUsers>);
  Register("lslong", new Creator<Lslong>);
  Register("hidden_files", new Creator<HiddenFiles>);
  Register("noretrieve", new Creator<Noretrieve>);
  Register("tagline", new Creator<Tagline>);
  Register("email", new Creator<Email>);
  Register("multiplier_max", new Creator<MultiplierMax>);
  Register("oneliners", new Creator<Oneliners>);
  Register("requests", new Creator<Requests>);
  Register("lastonline", new Creator<Lastonline>);
  Register("empty_nuke", new Creator<EmptyNuke>);
  Register("nodupecheck", new Creator<Nodupecheck>);
  Register("creditcheck", new Creator<Creditcheck>);
  Register("creditloss", new Creator<Creditloss>);
  Register("nukedir_style", new Creator<NukedirStyle>);
  Register("hideuser", new Creator<Hideuser>);
  Register("privgroup", new Creator<Privgroup>);
  Register("msgpath", new Creator<Msgpath>);
  Register("privpath", new Creator<Privpath>);
  Register("site_cmd", new Creator<SiteCmd>);
  Register("max_sitecmd_lines", new Creator<MaxSitecmdLines>);
  Register("cscript", new Creator<Cscript>);
}

setting::Setting *Create(const std::string& name)
{
  return registry.at(name)->Create();
}

}
