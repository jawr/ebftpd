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

  // adders
  

  // containers
  std::vector<AsciiDownloads*> asciiDownloads;
  std::vector<Shutdown*> shutdown;
  std::vector<FreeSpace*> freeSpace;
  std::vector<UseDirSize*> useDirSize;
  std::vector<Timezone*> timezone;
  std::vector<ColorMode*> colorMode;
  std::vector<SitenameLong*> sitenameLong;
  std::vector<SitenameShort*> sitenameShort;
  std::vector<LoginPrompt*> loginPrompt;
  std::vector<RootPath*> rootPath;
  std::vector<ReloadConfig*> reloadConfig;
  std::vector<Master*> master;
  std::vector<SecureIp*> secureIp;
  std::vector<SecurePass*> securePass;
  std::vector<DataPath*> dataPath;
  std::vector<PwdPath*> pwdPath;
  std::vector<GrpPath*> grpPath;
  std::vector<BotscriptPath*> botscriptPath;
  std::vector<BouncerIp*> bouncerIp;
  std::vector<SpeedLimit*> speedLimit;
  std::vector<SimXfers*> simXfers;
  std::vector<CalcCrc*> calcCrc;
  std::vector<Xdupe*> xdupe;
  std::vector<MmapAmount*> mmapAmount;
  std::vector<DlSendfile*> dlSendfile;
  std::vector<UlBufferedForce*> ulBufferedForce;
  std::vector<MinHomedir*> minHomedir;
  std::vector<ValidIp*> validIp;
  std::vector<ActiveAddr*> activeAddr;
  std::vector<PasvAddr*> pasvAddr;
  std::vector<Ports*> activePorts;
  std::vector<Ports*> pasvPorts;
  std::vector<AllowFxp*> allowFxp;
  std::vector<WelcomeMsg*> welcomeMsg;
  std::vector<GoodbyeMsg*> goodbyeMsg;
  std::vector<Newsfile*> newsfile;
  std::vector<Banner*> banner;
  std::vector<Alias*> alias;
  std::vector<Cdpath*> cdpath;
  std::vector<IgnoreType*> ignoreType;
  // ind rights
  std::vector<Right*> delete_; // delete is reserved
  std::vector<Right*> deleteown;
  std::vector<Right*> overwrite;
  std::vector<Right*> resume;
  std::vector<Right*> rename;
  std::vector<Right*> renameown;
  std::vector<Right*> filemove;
  std::vector<Right*> makedir;
  std::vector<Right*> upload;
  std::vector<Right*> download;
  std::vector<Right*> nuke;
  std::vector<Right*> dirlog;
  std::vector<Right*> hideinwho;
  std::vector<Right*> freefile;
  std::vector<Right*> nostats;
  // end rights
  std::vector<StatSection*> statSection;
  std::vector<PathFilter*> pathFilter;
  std::vector<MaxUsers*> maxUsers;
  std::vector<MaxUstats*> maxUstats;
  std::vector<MaxGstats*> maxGstats;
  std::vector<BannedUsers*> bannedUsers;
  std::vector<ShowDiz*> showDiz;
  std::vector<ShowTotals*> showTotals;
  std::vector<DlIncomplete*> dlIncomplete;
  std::vector<FileDlCount*> fileDlCount;
  std::vector<DupeCheck*> dupeCheck;
  std::vector<Script*> script;
  std::vector<IdleCommands*> idleCommands;
  std::vector<TotalUsers*> totalUsers;
  std::vector<Lslong*> lslong;
  std::vector<HiddenFiles*> hiddenFiles;
  std::vector<Noretrieve*> noretrieve;
  std::vector<Tagline*> tagline;
  std::vector<Email*> email;
  std::vector<MultiplierMax*> multiplierMax;
  std::vector<Oneliners*> oneliners;
  std::vector<Requests*> requests;
  std::vector<Lastonline*> lastonline;
  std::vector<EmptyNuke*> emptyNuke;
  std::vector<Nodupecheck*> nodupecheck;
  std::vector<Creditcheck*> creditcheck;
  std::vector<Creditloss*> creditloss;
  std::vector<NukedirStyle*> nukedirStyle;
  std::vector<Hideuser*> hideuser;
  std::vector<Privgroup*> privgroup;
  std::vector<Msgpath*> msgpath;
  std::vector<Privpath*> privpath;
  std::vector<SiteCmd*> siteCmd;
  std::vector<MaxSitecmdLines*> maxSitecmdLines;
  std::vector<Cscript*> cscript;

  // converter
  template <typename T> T *Convert(Setting* s) { return (T*)s; };

  // adders
  void AddAsciiDownloads(Setting* s) { asciiDownloads.push_back(Convert<AsciiDownloads>(s)); };
  void AddShutdown(Setting* s) { shutdown.push_back(Convert<Shutdown>(s)); };
  void AddFreeSpace(Setting* s) { freeSpace.push_back(Convert<FreeSpace>(s)); };
  void AddUseDirSize(Setting* s) { dirSize.push_back(Convert<UseDirSize>(s)); };
  void AddTimezone(Setting* s) { timezone.push_back(Convert<Timezone>(s)); };
  void AddColorMode(Setting* s) { colorMode.push_back(Convert<ColorMode>(s)); };
  void AddSitenameLong(Setting* s) { sitenameLong.push_back(Convert<SitenameLong>(s)); };
  void AddSitenameShort(Setting* s) { sitenameShort.push_back(Convert<SitenameShort>(s)); };
  void AddLoginPrompt(Setting* s) { loginPrompt.push_back(Convert<LoginPrompt>(s)); };
  void AddRootPath(Setting* s) { rootPath.push_back(Convert<RootPath>(s)); };
  void AddReloadConfig(Setting* s) { reloadConfig.push_back(Convert<ReloadConfig>(s)); };
  void AddMaster(Setting* s) { master.push_back(Convert<Master>(s)); };
  void AddSecureIp(Setting* s) { secureIp.push_back(Convert<SecureIp>(s)); };
  void AddSecurePass(Setting* s) { securePass.push_back(Convert<SecurePass>(s)); };
  void AddDataPath(Setting* s) { dataPath.push_back(Convert<DataPath>(s)); };
  void AddPwdPath(Setting* s) { pwdPath.push_back(Convert<PwdPath>(s)); };
  void AddGrpPath(Setting* s) { grpPath.push_back(Convert<GrpPath>(s)); };
  void AddBotscriptPath(Setting* s) { botscriptPath.push_back(Convert<BotscriptPath>(s)); };
  void AddBouncerIp(Setting* s) { bouncerIp.push_back(Convert<BouncerIp>(s)); };
  void AddSpeedLimit(Setting* s) { speedLimit.push_back(Convert<SpeedLimit>(s)); };
  void AddSimXfers(Setting* s) { simXfers.push_back(Convert<SimXfers>(s)); };
  void AddCalcCrc(Setting* s) { calcCrc.push_back(Convert<CalcCrc>(s)); };
  void AddXdupe(Setting* s) { xdupe.push_back(Convert<Xdupe>(s)); };
  void AddMmapAmount(Setting* s) { mmapAmount.push_back(Convert<MmapAmount>(s)); };
  void AddDlSendfile(Setting* s) { dlSendfile.push_back(Convert<DlSendfile>(s)); };
  void AddUlBufferedForce(Setting* s) { ulBufferedForce.push_back(Convert<UlBufferedForce>(s)); };
  void AddMinHomedir(Setting* s) { minHomedir.push_back(Convert<MinHomedir>(s)); };
  void AddValidIp(Setting* s) { validIp.push_back(Convert<ValidIp>(s)); };
  void AddActiveAddr(Setting* s) { activeAddr.push_back(Convert<ActiveAddr>(s)); };
  void AddPasvAddr(Setting* s) { pasvAddr.push_back(Convert<PasvAddr>(s)); };
  void AddActivePorts(Setting* s) { activePorts.push_back(Convert<Ports>(s)); };
  void AddPasvPorts(Setting* s) { pasvPorts.push_back(Convert<Ports>(s)); };
  void AddAllowFxp(Setting* s) { allowFxp.push_back(Convert<AllowFxp>(s)); };
  void AddWelcomeMsg(Setting* s) { welcomeMsg.push_back(Convert<WelcomeMsg>(s)); };
  void AddGoodbyeMsg(Setting* s) { goodbyeMsg.push_back(Convert<GoodbyeMsg>(s)); };
  void AddNewsfile(Setting* s) { newsfile.push_back(Convert<Newsfile>(s)); };
  void AddBanner(Setting* s) { banner.push_back(Convert<Banner>(s)); };
  void AddAlias(Setting* s) { alias.push_back(Convert<Alias>(s)); };
  void AddCdpath(Setting* s) { cdpath.push_back(Convert<Cdpath>(s)); };
  void AddIgnoreType(Setting* s) { ignoreType.push_back(Convert<IgnoreType>(s)); };
  // rights
  void AddDelete(Setting* s) { delete_.push_back(Convert<Right>(s)); };
  void AddDeleteown(Setting* s) { deleteown.push_back(Convert<Right>(s)); };
  void AddOverwrite(Setting* s) { overwrite.push_back(Convert<Right>(s)); };
  void AddResume(Setting* s) { resume.push_back(Convert<Right>(s)); };
  void AddRename(Setting* s) { rename.push_back(Convert<Right>(s)); };
  void AddRenameown(Setting* s) { renameown.push_back(Convert<Right>(s)); };
  void AddFilemove(Setting* s) { filemove.push_back(Convert<Right>(s)); };
  void AddMakedir(Setting* s) { makedir.push_back(Convert<Right>(s)); };
  void AddUpload(Setting* s) { upload.push_back(Convert<Right>(s)); };
  void AddDownload(Setting* s) { download.push_back(Convert<Right>(s)); };
  void AddNuke(Setting* s) { nuke.push_back(Convert<Right>(s)); };
  void AddDirlog(Setting* s) { dirlog.push_back(Convert<Right>(s)); };
  void AddHideinwho(Setting* s) { hideinwho.push_back(Convert<Right>(s)); };
  void AddFreefile(Setting* s) { freefile.push_back(Convert<Right>(s)); };
  void AddNostats(Setting* s) { nostats.push_back(Convert<Right>(s)); };
  // end rights section
  void AddStatSection(Setting* s) { statSection.push_back(Convert<StatSection>(s)); };
  void AddPathFilter(Setting* s) { pathFilter.push_back(Convert<PathFilter>(s)); };
  void AddMaxUsers(Setting* s) { maxUsers.push_back(Convert<MaxUsers>(s)); };
  void AddMaxUstats(Setting* s) { maxUstats.push_back(Convert<MaxUstats>(s)); };
  void AddMaxGstats(Setting* s) { maxGstats.push_back(Convert<MaxGstats>(s)); };
  void AddBannedUsers(Setting* s) { bannedUsers.push_back(Convert<BannedUsers>(s)); };
  void AddShowDiz(Setting* s) { showDiz.push_back(Convert<ShowDiz>(s)); };
  void AddShowTotals(Setting* s) { showTotals.push_back(Convert<ShowTotals>(s)); };
  void AddDlIncomplete(Setting* s) { dlIncomplete.push_back(Convert<DlIncomplete>(s)); };
  void AddFileDlCount(Setting* s) { fileDlCount.push_back(Convert<FileDlCount>(s)); };
  void AddDupeCheck(Setting* s) { dupeCheck.push_back(Convert<DupeCheck>(s)); };
  void AddScript(Setting* s) { script.push_back(Convert<Script>(s)); };
  void AddIdleCommands(Setting* s) { idleCommands.push_back(Convert<IdleCommands>(s)); };
  void AddTotalUsers(Setting* s) { totalUsers.push_back(Convert<TotalUsers>(s)); };
  void AddLslong(Setting* s) { lslong.push_back(Convert<Lslong>(s)); };
  void AddHiddenFiles(Setting* s) { hiddenFiles.push_back(Convert<HiddenFiles>(s)); };
  void AddNoretrieve(Setting* s) { noretrieve.push_back(Convert<Noretrieve>(s)); };
  void AddTagline(Setting* s) { tagline.push_back(Convert<Tagline>(s)); };
  void AddEmail(Setting* s) { email.push_back(Convert<Email>(s)); };
  void AddMultiplierMax(Setting* s) { multiplierMax.push_back(Convert<MultiplierMax>(s)); };
  void AddOneliners(Setting* s) { oneliners.push_back(Convert<Oneliners>(s)); };
  void AddRequests(Setting* s) { requests.push_back(Convert<Requests>(s)); };
  void AddLastonline(Setting* s) { lastonline.push_back(Convert<Lastonline>(s)); };
  void AddEmptyNuke(Setting* s) { emptyNuke.push_back(Convert<EmptyNuke>(s)); };
  void AddNodupecheck(Setting* s) { nodupecheck.push_back(Convert<Nodupecheck>(s)); };
  void AddCreditcheck(Setting* s) { creditcheck.push_back(Convert<Creditcheck>(s)); };
  void AddCreditloss(Setting* s) { creditloss.push_back(Convert<Creditloss>(s)); };
  void AddNukedirStyle(Setting* s) { nukedirStyle.push_back(Convert<NukedirStyle>(s)); };
  void AddHideuser(Setting* s) { hideuser.push_back(Convert<Hideuser>(s)); };
  void AddPrivgroup(Setting* s) { privgroup.push_back(Convert<Privgroup>(s)); };
  void AddMsgpath(Setting* s) { msgpath.push_back(Convert<Msgpath>(s)); };
  void AddPrivpath(Setting* s) { privpath.push_back(Convert<Privpath>(s)); };
  void AddSiteCmd(Setting* s) { siteCmd.push_back(Convert<SiteCmd>(s)); };
  void AddMaxSitecmdLines(Setting* s) { maxSitecmdLines .push_back(Convert<MaxSitecmdLines>(s)); };
  void AddCscript(Setting* s) { cscript.push_back(Convert<Cscript>(s)); };


public:
  Config(const std::string& configFile);
  ~Config() {};  

  int Version() const { return version; };

  // getters
  const std::vector<AsciiDownloads>& AsciiDownloads() const { return asciiDownloads; }; 
  const std::vector<Shutdown>& Shutdown() const { return shutdown; }; 
  const std::vector<FreeSpace>& FreeSpace() const { return freeSpace; }; 
  const std::vector<UseDirSize>& UseDirSize() const { return useDirSize; }; 
  const std::vector<Timezone>& Timezone() const { return timezone; }; 
  const std::vector<ColorMode>& ColorMode() const { return colorMode; }; 
  const std::vector<SitenameLong>& SitenameLong() const { return sitenameLong; }; 
  const std::vector<SitenameShort>& SitenameShort() const { return sitenameShort; }; 
  const std::vector<LoginPrompt>& LoginPrompt() const { return loginPrompt; }; 
  const std::vector<RootPath>& RootPath() const { return rootPath; }; 
  const std::vector<ReloadConfig>& ReloadConfig() const { return reloadConfig; }; 
  const std::vector<Master>& Master() const { return master; }; 
  const std::vector<SecureIp>& SecureIp() const { return secureIp; }; 
  const std::vector<SecurePass>& SecurePass() const { return securePass; }; 
  const std::vector<DataPath>& DataPath() const { return dataPath; }; 
  const std::vector<PwdPath>& PwdPath() const { return pwdPath; }; 
  const std::vector<GrpPath>& GrpPath() const { return grpPath; }; 
  const std::vector<BotscriptPath>& BotscriptPath() const { return botscriptPath; }; 
  const std::vector<BouncerIp>& BouncerIp() const { return bouncerIp; }; 
  const std::vector<SpeedLimit>& SpeedLimit() const { return speedLimit; }; 
  const std::vector<SimXfers>& SimXfers() const { return simXfers; }; 
  const std::vector<CalcCrc>& CalcCrc() const { return calcCrc; }; 
  const std::vector<Xdupe>& Xdupe() const { return xdupe; }; 
  const std::vector<MmapAmount>& MmapAmount() const { return mmapAmount; }; 
  const std::vector<DlSendfile>& DlSendfile() const { return dlSendfile; }; 
  const std::vector<UlBufferedForce>& UlBufferedForce() const { return ulBufferedForce; }; 
  const std::vector<MinHomedir>& MinHomedir() const { return minHomedir; }; 
  const std::vector<ValidIp>& ValidIp() const { return validIp; }; 
  const std::vector<ActiveAddr>& ActiveAddr() const { return activeAddr; }; 
  const std::vector<PasvAddr>& PasvAddr() const { return pasvAddr; }; 
  const std::vector<Ports>& ActivePorts() const { return activePorts; }; 
  const std::vector<Ports>& PasvPorts() const { return pasvPorts; }; 
  const std::vector<AllowFxp>& AllowFxp() const { return allowFxp; }; 
  const std::vector<WelcomeMsg>& WelcomeMsg() const { return welcomeMsg; }; 
  const std::vector<GoodbyeMsg>& GoodbyeMsg() const { return goodbyeMsg; }; 
  const std::vector<Newsfile>& Newsfile() const { return newsfile; }; 
  const std::vector<Banner>& Banner() const { return banner; }; 
  const std::vector<Alias>& Alias() const { return alias; }; 
  const std::vector<Cdpath>& Cdpath() const { return cdpath; }; 
  const std::vector<IgnoreType>& IgnoreType() const { return ignoreType; }; 
  // rights section
  const std::vector<Right>& Delete() const { return delete_; }; 
  const std::vector<Right>& Deleteown() const { return deleteown; }; 
  const std::vector<Right>& Overwrite() const { return overwrite; }; 
  const std::vector<Right>& Resume() const { return resume; }; 
  const std::vector<Right>& Rename() const { return rename; }; 
  const std::vector<Right>& Renameown() const { return renameown; }; 
  const std::vector<Right>& Filemove() const { return filemove; }; 
  const std::vector<Right>& Makedir() const { return makedir; }; 
  const std::vector<Right>& Upload() const { return upload; }; 
  const std::vector<Right>& Download() const { return download; }; 
  const std::vector<Right>& Nuke() const { return nuke; }; 
  const std::vector<Right>& Dirlog() const { return dirlog; }; 
  const std::vector<Right>& Hideinwho() const { return hideinwho; }; 
  const std::vector<Right>& Freefile() const { return freefile; }; 
  // end rights section
  const std::vector<StatSection>& StatSection() const { return statSection; }; 
  const std::vector<PathFilter>& PathFilter() const { return pathFilter; }; 
  const std::vector<MaxUsers>& MaxUsers() const { return maxUsers; }; 
  const std::vector<MaxUstats>& MaxUstats() const { return maxUstats; }; 
  const std::vector<MaxGstats>& MaxGstats() const { return maxGstats; }; 
  const std::vector<BannedUsers>& BannedUsers() const { return bannedUsers; }; 
  const std::vector<ShowDiz>& ShowDiz() const { return showDiz; }; 
  const std::vector<ShowTotals>& ShowTotals() const { return showTotals; }; 
  const std::vector<DlIncomplete>& DlIncomplete() const { return dlIncomplete; }; 
  const std::vector<FileDlCount>& FileDlCount() const { return fileDlCount; }; 
  const std::vector<DupeCheck>& DupeCheck() const { return dupeCheck; }; 
  const std::vector<Script>& Script() const { return script; }; 
  const std::vector<IdleCommands>& IdleCommands() const { return idleCommands; }; 
  const std::vector<TotalUsers>& TotalUsers() const { return totalUsers; }; 
  const std::vector<Lslong>& Lslong() const { return lslong; }; 
  const std::vector<HiddenFiles>& HiddenFiles() const { return hiddenFiles; }; 
  const std::vector<Noretrieve>& Noretrieve() const { return noretrieve; }; 
  const std::vector<Tagline>& Tagline() const { return tagline; }; 
  const std::vector<Email>& Email() const { return email; }; 
  const std::vector<MultiplierMax>& MultiplierMax() const { return multiplierMax; }; 
  const std::vector<Oneliners>& Oneliners() const { return oneliners; }; 
  const std::vector<Requests>& Requests() const { return requests; }; 
  const std::vector<Lastonline>& Lastonline() const { return lastonline; }; 
  const std::vector<EmptyNuke>& EmptyNuke() const { return emptyNuke; }; 
  const std::vector<Nodupecheck>& Nodupecheck() const { return nodupecheck; }; 
  const std::vector<Creditcheck>& Creditcheck() const { return creditcheck; }; 
  const std::vector<Creditloss>& Creditloss() const { return creditloss; }; 
  const std::vector<NukedirStyle>& NukedirStyle() const { return nukedirStyle; }; 
  const std::vector<Hideuser>& Hideuser() const { return hideuser; }; 
  const std::vector<Privgroup>& Privgroup() const { return privgroup; }; 
  const std::vector<Msgpath>& Msgpath() const { return msgpath; }; 
  const std::vector<Privpath>& Privpath() const { return privpath; }; 
  const std::vector<SiteCmd>& SiteCmd() const { return siteCmd; }; 
  const std::vector<MaxSitecmdLines>& MaxSitecmdLines() const { return maxSitecmdLines; }; 
  const std::vector<Cscript>& Cscript() const { return cscript; }; 

};

typedef std::tr1::shared_ptr<cfg::Config> ConfigPtr;

}


#endif 
