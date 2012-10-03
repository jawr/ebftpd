#ifndef __CFG_CONFIG_CPP
#define __CFG_CONFIG_CPP
#include <vector>
#include <string>
#include <boost/unordered_map.hpp>
#include <tr1/memory>
#include "cfg/exception.hpp"
#include "cfg/setting.hpp"
#include "fs/path.hpp"


namespace cfg
{

class Config
{ 
  static int latestVersion;
  int version;

  std::string config;
 
  void Parse(const std::string& line);

  // containers
  std::vector<setting::AsciiDownloads*> asciiDownloads;
  std::vector<setting::Shutdown*> shutdown;
  std::vector<setting::FreeSpace*> freeSpace;
  std::vector<setting::UseDirSize*> useDirSize;
  std::vector<setting::Timezone*> timezone;
  std::vector<setting::ColorMode*> colorMode;
  std::vector<setting::SitenameLong*> sitenameLong;
  std::vector<setting::SitenameShort*> sitenameShort;
  std::vector<setting::LoginPrompt*> loginPrompt;
  std::vector<setting::RootPath*> rootPath;
  std::vector<setting::ReloadConfig*> reloadConfig;
  std::vector<setting::Master*> master;
  std::vector<setting::SecureIp*> secureIp;
  std::vector<setting::SecurePass*> securePass;
  std::vector<setting::DataPath*> dataPath;
  std::vector<setting::PwdPath*> pwdPath;
  std::vector<setting::GrpPath*> grpPath;
  std::vector<setting::BotscriptPath*> botscriptPath;
  std::vector<setting::BouncerIp*> bouncerIp;
  std::vector<setting::SpeedLimit*> speedLimit;
  std::vector<setting::SimXfers*> simXfers;
  std::vector<setting::CalcCrc*> calcCrc;
  std::vector<setting::Xdupe*> xdupe;
  std::vector<setting::MmapAmount*> mmapAmount;
  std::vector<setting::DlSendfile*> dlSendfile;
  std::vector<setting::UlBufferedForce*> ulBufferedForce;
  std::vector<setting::MinHomedir*> minHomedir;
  std::vector<setting::ValidIp*> validIp;
  std::vector<setting::ActiveAddr*> activeAddr;
  std::vector<setting::PasvAddr*> pasvAddr;
  std::vector<setting::Ports*> activePorts;
  std::vector<setting::Ports*> pasvPorts;
  std::vector<setting::AllowFxp*> allowFxp;
  std::vector<setting::WelcomeMsg*> welcomeMsg;
  std::vector<setting::GoodbyeMsg*> goodbyeMsg;
  std::vector<setting::Newsfile*> newsfile;
  std::vector<setting::Banner*> banner;
  std::vector<setting::Alias*> alias;
  std::vector<setting::Cdpath*> cdpath;
  std::vector<setting::IgnoreType*> ignoreType;
  // ind rights
  std::vector<setting::Right*> delete_; // delete is reserved
  std::vector<setting::Right*> deleteown;
  std::vector<setting::Right*> overwrite;
  std::vector<setting::Right*> resume;
  std::vector<setting::Right*> rename;
  std::vector<setting::Right*> renameown;
  std::vector<setting::Right*> filemove;
  std::vector<setting::Right*> makedir;
  std::vector<setting::Right*> upload;
  std::vector<setting::Right*> download;
  std::vector<setting::Right*> nuke;
  std::vector<setting::Right*> dirlog;
  std::vector<setting::Right*> hideinwho;
  std::vector<setting::Right*> freefile;
  std::vector<setting::Right*> nostats;
  // end rights
  std::vector<setting::StatSection*> statSection;
  std::vector<setting::PathFilter*> pathFilter;
  std::vector<setting::MaxUsers*> maxUsers;
  std::vector<setting::MaxUstats*> maxUstats;
  std::vector<setting::MaxGstats*> maxGstats;
  std::vector<setting::BannedUsers*> bannedUsers;
  std::vector<setting::ShowDiz*> showDiz;
  std::vector<setting::ShowTotals*> showTotals;
  std::vector<setting::DlIncomplete*> dlIncomplete;
  std::vector<setting::FileDlCount*> fileDlCount;
  std::vector<setting::DupeCheck*> dupeCheck;
  std::vector<setting::Script*> script;
  std::vector<setting::IdleCommands*> idleCommands;
  std::vector<setting::TotalUsers*> totalUsers;
  std::vector<setting::Lslong*> lslong;
  std::vector<setting::HiddenFiles*> hiddenFiles;
  std::vector<setting::Noretrieve*> noretrieve;
  std::vector<setting::Tagline*> tagline;
  std::vector<setting::Email*> email;
  std::vector<setting::MultiplierMax*> multiplierMax;
  std::vector<setting::Oneliners*> oneliners;
  std::vector<setting::Requests*> requests;
  std::vector<setting::Lastonline*> lastonline;
  std::vector<setting::EmptyNuke*> emptyNuke;
  std::vector<setting::Nodupecheck*> nodupecheck;
  std::vector<setting::Creditcheck*> creditcheck;
  std::vector<setting::Creditloss*> creditloss;
  std::vector<setting::NukedirStyle*> nukedirStyle;
  std::vector<setting::Hideuser*> hideuser;
  std::vector<setting::Privgroup*> privgroup;
  std::vector<setting::Msgpath*> msgpath;
  std::vector<setting::Privpath*> privpath;
  std::vector<setting::SiteCmd*> siteCmd;
  std::vector<setting::MaxSitecmdLines*> maxSitecmdLines;
  std::vector<setting::Cscript*> cscript;

  // converter
  template <typename T> T *Convert(setting::Setting* s) { return (T*)s; };

  // adders
  void AddAsciiDownloads(setting::Setting* s) { asciiDownloads.push_back(Convert<setting::AsciiDownloads>(s)); };
  void AddShutdown(setting::Setting* s) { shutdown.push_back(Convert<setting::Shutdown>(s)); };
  void AddFreeSpace(setting::Setting* s) { freeSpace.push_back(Convert<setting::FreeSpace>(s)); };
  void AddUseDirSize(setting::Setting* s) { useDirSize.push_back(Convert<setting::UseDirSize>(s)); };
  void AddTimezone(setting::Setting* s) { timezone.push_back(Convert<setting::Timezone>(s)); };
  void AddColorMode(setting::Setting* s) { colorMode.push_back(Convert<setting::ColorMode>(s)); };
  void AddSitenameLong(setting::Setting* s) { sitenameLong.push_back(Convert<setting::SitenameLong>(s)); };
  void AddSitenameShort(setting::Setting* s) { sitenameShort.push_back(Convert<setting::SitenameShort>(s)); };
  void AddLoginPrompt(setting::Setting* s) { loginPrompt.push_back(Convert<setting::LoginPrompt>(s)); };
  void AddRootPath(setting::Setting* s) { rootPath.push_back(Convert<setting::RootPath>(s)); };
  void AddReloadConfig(setting::Setting* s) { reloadConfig.push_back(Convert<setting::ReloadConfig>(s)); };
  void AddMaster(setting::Setting* s) { master.push_back(Convert<setting::Master>(s)); };
  void AddSecureIp(setting::Setting* s) { secureIp.push_back(Convert<setting::SecureIp>(s)); };
  void AddSecurePass(setting::Setting* s) { securePass.push_back(Convert<setting::SecurePass>(s)); };
  void AddDataPath(setting::Setting* s) { dataPath.push_back(Convert<setting::DataPath>(s)); };
  void AddPwdPath(setting::Setting* s) { pwdPath.push_back(Convert<setting::PwdPath>(s)); };
  void AddGrpPath(setting::Setting* s) { grpPath.push_back(Convert<setting::GrpPath>(s)); };
  void AddBotscriptPath(setting::Setting* s) { botscriptPath.push_back(Convert<setting::BotscriptPath>(s)); };
  void AddBouncerIp(setting::Setting* s) { bouncerIp.push_back(Convert<setting::BouncerIp>(s)); };
  void AddSpeedLimit(setting::Setting* s) { speedLimit.push_back(Convert<setting::SpeedLimit>(s)); };
  void AddSimXfers(setting::Setting* s) { simXfers.push_back(Convert<setting::SimXfers>(s)); };
  void AddCalcCrc(setting::Setting* s) { calcCrc.push_back(Convert<setting::CalcCrc>(s)); };
  void AddXdupe(setting::Setting* s) { xdupe.push_back(Convert<setting::Xdupe>(s)); };
  void AddMmapAmount(setting::Setting* s) { mmapAmount.push_back(Convert<setting::MmapAmount>(s)); };
  void AddDlSendfile(setting::Setting* s) { dlSendfile.push_back(Convert<setting::DlSendfile>(s)); };
  void AddUlBufferedForce(setting::Setting* s) { ulBufferedForce.push_back(Convert<setting::UlBufferedForce>(s)); };
  void AddMinHomedir(setting::Setting* s) { minHomedir.push_back(Convert<setting::MinHomedir>(s)); };
  void AddValidIp(setting::Setting* s) { validIp.push_back(Convert<setting::ValidIp>(s)); };
  void AddActiveAddr(setting::Setting* s) { activeAddr.push_back(Convert<setting::ActiveAddr>(s)); };
  void AddPasvAddr(setting::Setting* s) { pasvAddr.push_back(Convert<setting::PasvAddr>(s)); };
  void AddActivePorts(setting::Setting* s) { activePorts.push_back(Convert<setting::Ports>(s)); };
  void AddPasvPorts(setting::Setting* s) { pasvPorts.push_back(Convert<setting::Ports>(s)); };
  void AddAllowFxp(setting::Setting* s) { allowFxp.push_back(Convert<setting::AllowFxp>(s)); };
  void AddWelcomeMsg(setting::Setting* s) { welcomeMsg.push_back(Convert<setting::WelcomeMsg>(s)); };
  void AddGoodbyeMsg(setting::Setting* s) { goodbyeMsg.push_back(Convert<setting::GoodbyeMsg>(s)); };
  void AddNewsfile(setting::Setting* s) { newsfile.push_back(Convert<setting::Newsfile>(s)); };
  void AddBanner(setting::Setting* s) { banner.push_back(Convert<setting::Banner>(s)); };
  void AddAlias(setting::Setting* s) { alias.push_back(Convert<setting::Alias>(s)); };
  void AddCdpath(setting::Setting* s) { cdpath.push_back(Convert<setting::Cdpath>(s)); };
  void AddIgnoreType(setting::Setting* s) { ignoreType.push_back(Convert<setting::IgnoreType>(s)); };
  // rights
  void AddDelete(setting::Setting* s) { delete_.push_back(Convert<setting::Right>(s)); };
  void AddDeleteown(setting::Setting* s) { deleteown.push_back(Convert<setting::Right>(s)); };
  void AddOverwrite(setting::Setting* s) { overwrite.push_back(Convert<setting::Right>(s)); };
  void AddResume(setting::Setting* s) { resume.push_back(Convert<setting::Right>(s)); };
  void AddRename(setting::Setting* s) { rename.push_back(Convert<setting::Right>(s)); };
  void AddRenameown(setting::Setting* s) { renameown.push_back(Convert<setting::Right>(s)); };
  void AddFilemove(setting::Setting* s) { filemove.push_back(Convert<setting::Right>(s)); };
  void AddMakedir(setting::Setting* s) { makedir.push_back(Convert<setting::Right>(s)); };
  void AddUpload(setting::Setting* s) { upload.push_back(Convert<setting::Right>(s)); };
  void AddDownload(setting::Setting* s) { download.push_back(Convert<setting::Right>(s)); };
  void AddNuke(setting::Setting* s) { nuke.push_back(Convert<setting::Right>(s)); };
  void AddDirlog(setting::Setting* s) { dirlog.push_back(Convert<setting::Right>(s)); };
  void AddHideinwho(setting::Setting* s) { hideinwho.push_back(Convert<setting::Right>(s)); };
  void AddFreefile(setting::Setting* s) { freefile.push_back(Convert<setting::Right>(s)); };
  void AddNostats(setting::Setting* s) { nostats.push_back(Convert<setting::Right>(s)); };
  // end rights section
  void AddStatSection(setting::Setting* s) { statSection.push_back(Convert<setting::StatSection>(s)); };
  void AddPathFilter(setting::Setting* s) { pathFilter.push_back(Convert<setting::PathFilter>(s)); };
  void AddMaxUsers(setting::Setting* s) { maxUsers.push_back(Convert<setting::MaxUsers>(s)); };
  void AddMaxUstats(setting::Setting* s) { maxUstats.push_back(Convert<setting::MaxUstats>(s)); };
  void AddMaxGstats(setting::Setting* s) { maxGstats.push_back(Convert<setting::MaxGstats>(s)); };
  void AddBannedUsers(setting::Setting* s) { bannedUsers.push_back(Convert<setting::BannedUsers>(s)); };
  void AddShowDiz(setting::Setting* s) { showDiz.push_back(Convert<setting::ShowDiz>(s)); };
  void AddShowTotals(setting::Setting* s) { showTotals.push_back(Convert<setting::ShowTotals>(s)); };
  void AddDlIncomplete(setting::Setting* s) { dlIncomplete.push_back(Convert<setting::DlIncomplete>(s)); };
  void AddFileDlCount(setting::Setting* s) { fileDlCount.push_back(Convert<setting::FileDlCount>(s)); };
  void AddDupeCheck(setting::Setting* s) { dupeCheck.push_back(Convert<setting::DupeCheck>(s)); };
  void AddScript(setting::Setting* s) { script.push_back(Convert<setting::Script>(s)); };
  void AddIdleCommands(setting::Setting* s) { idleCommands.push_back(Convert<setting::IdleCommands>(s)); };
  void AddTotalUsers(setting::Setting* s) { totalUsers.push_back(Convert<setting::TotalUsers>(s)); };
  void AddLslong(setting::Setting* s) { lslong.push_back(Convert<setting::Lslong>(s)); };
  void AddHiddenFiles(setting::Setting* s) { hiddenFiles.push_back(Convert<setting::HiddenFiles>(s)); };
  void AddNoretrieve(setting::Setting* s) { noretrieve.push_back(Convert<setting::Noretrieve>(s)); };
  void AddTagline(setting::Setting* s) { tagline.push_back(Convert<setting::Tagline>(s)); };
  void AddEmail(setting::Setting* s) { email.push_back(Convert<setting::Email>(s)); };
  void AddMultiplierMax(setting::Setting* s) { multiplierMax.push_back(Convert<setting::MultiplierMax>(s)); };
  void AddOneliners(setting::Setting* s) { oneliners.push_back(Convert<setting::Oneliners>(s)); };
  void AddRequests(setting::Setting* s) { requests.push_back(Convert<setting::Requests>(s)); };
  void AddLastonline(setting::Setting* s) { lastonline.push_back(Convert<setting::Lastonline>(s)); };
  void AddEmptyNuke(setting::Setting* s) { emptyNuke.push_back(Convert<setting::EmptyNuke>(s)); };
  void AddNodupecheck(setting::Setting* s) { nodupecheck.push_back(Convert<setting::Nodupecheck>(s)); };
  void AddCreditcheck(setting::Setting* s) { creditcheck.push_back(Convert<setting::Creditcheck>(s)); };
  void AddCreditloss(setting::Setting* s) { creditloss.push_back(Convert<setting::Creditloss>(s)); };
  void AddNukedirStyle(setting::Setting* s) { nukedirStyle.push_back(Convert<setting::NukedirStyle>(s)); };
  void AddHideuser(setting::Setting* s) { hideuser.push_back(Convert<setting::Hideuser>(s)); };
  void AddPrivgroup(setting::Setting* s) { privgroup.push_back(Convert<setting::Privgroup>(s)); };
  void AddMsgpath(setting::Setting* s) { msgpath.push_back(Convert<setting::Msgpath>(s)); };
  void AddPrivpath(setting::Setting* s) { privpath.push_back(Convert<setting::Privpath>(s)); };
  void AddSiteCmd(setting::Setting* s) { siteCmd.push_back(Convert<setting::SiteCmd>(s)); };
  void AddMaxSitecmdLines(setting::Setting* s) { maxSitecmdLines .push_back(Convert<setting::MaxSitecmdLines>(s)); };
  void AddCscript(setting::Setting* s) { cscript.push_back(Convert<setting::Cscript>(s)); };


public:
  Config(const std::string& configFile);
  ~Config() {};  

  int Version() const { return version; };

  // getters
  const std::vector<setting::AsciiDownloads*>& AsciiDownloads() const { return asciiDownloads; }; 
  const std::vector<setting::Shutdown*>& Shutdown() const { return shutdown; }; 
  const std::vector<setting::FreeSpace*>& FreeSpace() const { return freeSpace; }; 
  const std::vector<setting::UseDirSize*>& UseDirSize() const { return useDirSize; }; 
  const std::vector<setting::Timezone*>& Timezone() const { return timezone; }; 
  const std::vector<setting::ColorMode*>& ColorMode() const { return colorMode; }; 
  const std::vector<setting::SitenameLong*>& SitenameLong() const { return sitenameLong; }; 
  const std::vector<setting::SitenameShort*>& SitenameShort() const { return sitenameShort; }; 
  const std::vector<setting::LoginPrompt*>& LoginPrompt() const { return loginPrompt; }; 
  const std::vector<setting::RootPath*>& RootPath() const { return rootPath; }; 
  const std::vector<setting::ReloadConfig*>& ReloadConfig() const { return reloadConfig; }; 
  const std::vector<setting::Master*>& Master() const { return master; }; 
  const std::vector<setting::SecureIp*>& SecureIp() const { return secureIp; }; 
  const std::vector<setting::SecurePass*>& SecurePass() const { return securePass; }; 
  const std::vector<setting::DataPath*>& DataPath() const { return dataPath; }; 
  const std::vector<setting::PwdPath*>& PwdPath() const { return pwdPath; }; 
  const std::vector<setting::GrpPath*>& GrpPath() const { return grpPath; }; 
  const std::vector<setting::BotscriptPath*>& BotscriptPath() const { return botscriptPath; }; 
  const std::vector<setting::BouncerIp*>& BouncerIp() const { return bouncerIp; }; 
  const std::vector<setting::SpeedLimit*>& SpeedLimit() const { return speedLimit; }; 
  const std::vector<setting::SimXfers*>& SimXfers() const { return simXfers; }; 
  const std::vector<setting::CalcCrc*>& CalcCrc() const { return calcCrc; }; 
  const std::vector<setting::Xdupe*>& Xdupe() const { return xdupe; }; 
  const std::vector<setting::MmapAmount*>& MmapAmount() const { return mmapAmount; }; 
  const std::vector<setting::DlSendfile*>& DlSendfile() const { return dlSendfile; }; 
  const std::vector<setting::UlBufferedForce*>& UlBufferedForce() const { return ulBufferedForce; }; 
  const std::vector<setting::MinHomedir*>& MinHomedir() const { return minHomedir; }; 
  const std::vector<setting::ValidIp*>& ValidIp() const { return validIp; }; 
  const std::vector<setting::ActiveAddr*>& ActiveAddr() const { return activeAddr; }; 
  const std::vector<setting::PasvAddr*>& PasvAddr() const { return pasvAddr; }; 
  const std::vector<setting::Ports*>& ActivePorts() const { return activePorts; }; 
  const std::vector<setting::Ports*>& PasvPorts() const { return pasvPorts; }; 
  const std::vector<setting::AllowFxp*>& AllowFxp() const { return allowFxp; }; 
  const std::vector<setting::WelcomeMsg*>& WelcomeMsg() const { return welcomeMsg; }; 
  const std::vector<setting::GoodbyeMsg*>& GoodbyeMsg() const { return goodbyeMsg; }; 
  const std::vector<setting::Newsfile*>& Newsfile() const { return newsfile; }; 
  const std::vector<setting::Banner*>& Banner() const { return banner; }; 
  const std::vector<setting::Alias*>& Alias() const { return alias; }; 
  const std::vector<setting::Cdpath*>& Cdpath() const { return cdpath; }; 
  const std::vector<setting::IgnoreType*>& IgnoreType() const { return ignoreType; }; 
  // rights section
  const std::vector<setting::Right*>& Delete() const { return delete_; }; 
  const std::vector<setting::Right*>& Deleteown() const { return deleteown; }; 
  const std::vector<setting::Right*>& Overwrite() const { return overwrite; }; 
  const std::vector<setting::Right*>& Resume() const { return resume; }; 
  const std::vector<setting::Right*>& Rename() const { return rename; }; 
  const std::vector<setting::Right*>& Renameown() const { return renameown; }; 
  const std::vector<setting::Right*>& Filemove() const { return filemove; }; 
  const std::vector<setting::Right*>& Makedir() const { return makedir; }; 
  const std::vector<setting::Right*>& Upload() const { return upload; }; 
  const std::vector<setting::Right*>& Download() const { return download; }; 
  const std::vector<setting::Right*>& Nuke() const { return nuke; }; 
  const std::vector<setting::Right*>& Dirlog() const { return dirlog; }; 
  const std::vector<setting::Right*>& Hideinwho() const { return hideinwho; }; 
  const std::vector<setting::Right*>& Freefile() const { return freefile; }; 
  // end rights section
  const std::vector<setting::StatSection*>& StatSection() const { return statSection; }; 
  const std::vector<setting::PathFilter*>& PathFilter() const { return pathFilter; }; 
  const std::vector<setting::MaxUsers*>& MaxUsers() const { return maxUsers; }; 
  const std::vector<setting::MaxUstats*>& MaxUstats() const { return maxUstats; }; 
  const std::vector<setting::MaxGstats*>& MaxGstats() const { return maxGstats; }; 
  const std::vector<setting::BannedUsers*>& BannedUsers() const { return bannedUsers; }; 
  const std::vector<setting::ShowDiz*>& ShowDiz() const { return showDiz; }; 
  const std::vector<setting::ShowTotals*>& ShowTotals() const { return showTotals; }; 
  const std::vector<setting::DlIncomplete*>& DlIncomplete() const { return dlIncomplete; }; 
  const std::vector<setting::FileDlCount*>& FileDlCount() const { return fileDlCount; }; 
  const std::vector<setting::DupeCheck*>& DupeCheck() const { return dupeCheck; }; 
  const std::vector<setting::Script*>& Script() const { return script; }; 
  const std::vector<setting::IdleCommands*>& IdleCommands() const { return idleCommands; }; 
  const std::vector<setting::TotalUsers*>& TotalUsers() const { return totalUsers; }; 
  const std::vector<setting::Lslong*>& Lslong() const { return lslong; }; 
  const std::vector<setting::HiddenFiles*>& HiddenFiles() const { return hiddenFiles; }; 
  const std::vector<setting::Noretrieve*>& Noretrieve() const { return noretrieve; }; 
  const std::vector<setting::Tagline*>& Tagline() const { return tagline; }; 
  const std::vector<setting::Email*>& Email() const { return email; }; 
  const std::vector<setting::MultiplierMax*>& MultiplierMax() const { return multiplierMax; }; 
  const std::vector<setting::Oneliners*>& Oneliners() const { return oneliners; }; 
  const std::vector<setting::Requests*>& Requests() const { return requests; }; 
  const std::vector<setting::Lastonline*>& Lastonline() const { return lastonline; }; 
  const std::vector<setting::EmptyNuke*>& EmptyNuke() const { return emptyNuke; }; 
  const std::vector<setting::Nodupecheck*>& Nodupecheck() const { return nodupecheck; }; 
  const std::vector<setting::Creditcheck*>& Creditcheck() const { return creditcheck; }; 
  const std::vector<setting::Creditloss*>& Creditloss() const { return creditloss; }; 
  const std::vector<setting::NukedirStyle*>& NukedirStyle() const { return nukedirStyle; }; 
  const std::vector<setting::Hideuser*>& Hideuser() const { return hideuser; }; 
  const std::vector<setting::Privgroup*>& Privgroup() const { return privgroup; }; 
  const std::vector<setting::Msgpath*>& Msgpath() const { return msgpath; }; 
  const std::vector<setting::Privpath*>& Privpath() const { return privpath; }; 
  const std::vector<setting::SiteCmd*>& SiteCmd() const { return siteCmd; }; 
  const std::vector<setting::MaxSitecmdLines*>& MaxSitecmdLines() const { return maxSitecmdLines; }; 
  const std::vector<setting::Cscript*>& Cscript() const { return cscript; }; 

};

typedef std::tr1::shared_ptr<cfg::Config> ConfigPtr;

}


#endif 
