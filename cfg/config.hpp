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

public:
  Config(const std::string& configFile);
  ~Config() {};  

  int Version() const { return version; };

};

typedef std::tr1::shared_ptr<cfg::Config> ConfigPtr;

}


#endif 
