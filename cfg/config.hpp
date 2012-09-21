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
  void SetSetting(const std::string& setting, std::vector<std::string>& toks);

  // containers
  boost::unordered_map<std::string, std::vector<fs::Path> > MapPath;
  boost::unordered_map<std::string, std::vector<setting::ACL> > MapACL;
  boost::unordered_map<std::string, std::vector<setting::IntWithArguments> > MapIntWithArguments;
  boost::unordered_map<std::string, std::vector<std::string> > MapStrings;
  boost::unordered_map<std::string, std::vector<int> > MapInt;
  boost::unordered_map<std::string, std::vector<bool> > MapBool; 
  boost::unordered_map<std::string, std::vector<setting::SecureIP> > MapSecureIP;
  boost::unordered_map<std::string, std::vector<setting::SpeedLimit> > MapSpeedLimit;
  boost::unordered_map<std::string, std::vector<setting::PasvAddr> > MapPasvAddr;
  boost::unordered_map<std::string, std::vector<setting::AllowFxp> > MapAllowFxp;
  boost::unordered_map<std::string, std::vector<setting::ACLWithPath> > MapACLWithPath;
  boost::unordered_map<std::string, std::vector<setting::PathWithArgument> > MapPathWithArgument;
  boost::unordered_map<std::string, std::vector<setting::ACLWithArgument> > MapACLWithArgument;
  boost::unordered_map<std::string, std::vector<setting::StatSection> > MapStatSection;
  boost::unordered_map<std::string, std::vector<setting::PathFilter> > MapPathFilter;
  boost::unordered_map<std::string, std::vector<setting::ACLWithInt> > MapACLWithInt;
  boost::unordered_map<std::string, std::vector<setting::IntWithBool> > MapIntWithBool;
  boost::unordered_map<std::string, std::vector<setting::Requests> > MapRequests; 
  boost::unordered_map<std::string, std::vector<setting::Creditcheck> > MapCreditcheck;
  boost::unordered_map<std::string, std::vector<setting::NukedirStyle> > MapNukedirStyle;
  boost::unordered_map<std::string, std::vector<setting::MsgPath> > MapMsgPath;

  // site commands
  boost::unordered_map<std::string, setting::SiteCmd> MapSiteCmd;

  // custom commands
  boost::unordered_map<std::string, std::vector<setting::Cscript> > MapCscript;
  

   
public:
  Config(const std::string& configFile);
  ~Config() {};  

  int Version() const { return version; };

  // getters
  // follows the following syntax:
  //  userrejectsecure -> Userrejectsecure
  //  ascii_downloads  -> AsciiDownloads 
  const fs::Path& DSACertFile();
  const setting::ACL& Userrejectsecure();
  const setting::ACL& Userrejectinsecure();
  const setting::ACL& Denydiruncrypted();
  const setting::ACL& Denydatauncrypted();
  const setting::IntWithArguments& AsciiDownloads();
  const setting::ACL& Shutdown();
  int FreeSpace();
  const std::vector<std::string>& UseDirSize();
  int Timezone();
  bool ColorMode();
  const std::string& SitenameLong();
  const std::string& SitenameShort();
  const std::string& LoginPrompt();
  const fs::Path& Rootpath();
  const fs::Path& Datapath();
  const fs::Path& ReloadConfig(); // needed?
  const std::vector<std::string>& Master();
  const std::vector<setting::SecureIP>& SecureIp();
  const std::vector<setting::ACLWithPath>& SecurePass();
  const fs::Path& PwdPath();
  const fs::Path& GrpPath();
  const fs::Path& BotscriptPath();
  const std::vector<std::string>& BouncerIp();
  const std::vector<setting::SpeedLimit>& SpeedLimit();
  const std::vector<int>& SimXfers();
  const fs::Path& CalcCrc();
  const std::vector<std::string>& Xdupe();
  int MmapAmount();
  int DlSendfile();
  int UlBufferedForce();
  const fs::Path& MinHomedir();
  const std::vector<std::string>& ValidIp();
  const std::vector<std::string>& ActiveAddr();
  const std::vector<setting::PasvAddr>& PasvAddr();
  const std::vector<std::string>& PasvPorts();
  const std::vector<std::string>& ActivePorts();
  const setting::AllowFxp& AllowFxp();
  const std::vector<setting::ACLWithPath>& WelcomeMsg();
  const std::vector<setting::ACLWithPath>& GoodbyeMsg();
  const std::vector<setting::ACLWithPath>& Newsfile();
  const std::vector<fs::Path>& Banner();
  const std::vector<setting::PathWithArgument>& Alias();
  const std::vector<std::string>& Cdpath();
  const std::vector<std::string>& IgnoreType();
  const std::vector<setting::ACLWithPath>& Delete();   
  const std::vector<setting::ACLWithPath>& Deleteown();   
  const std::vector<setting::ACLWithPath>& Overwrite();   
  const std::vector<setting::ACLWithPath>& Resume();   
  const std::vector<setting::ACLWithPath>& Rename();   
  const std::vector<setting::ACLWithPath>& Renameown();   
  const std::vector<setting::ACLWithPath>& Filemove();   
  const std::vector<setting::ACLWithPath>& Makedir();   
  const std::vector<setting::ACLWithPath>& Upload();   
  const std::vector<setting::ACLWithPath>& Download();   
  const std::vector<setting::ACLWithPath>& Nuke();   
  const std::vector<setting::ACLWithPath>& Dirlog();   
  const std::vector<setting::ACLWithPath>& Hideinwho();   
  const std::vector<setting::ACLWithPath>& Nostats();   
  const std::vector<setting::ACLWithArgument>& Freefile();
  const std::vector<setting::StatSection>& StatSection();
  const std::vector<setting::PathFilter>& PathFilter();
  const std::vector<int>& MaxUsers();
  const setting::ACLWithInt& MaxUstats(); 
  const setting::ACLWithInt& MaxGstats(); 
  const std::vector<std::string>& BannedUsers();
  const setting::ACLWithPath& ShowDiz();
  const setting::IntWithArguments& ShowTotals();
  bool DlIncomplete();
  bool FileDlCount();
  const setting::IntWithBool& DupeCheck();
  const setting::ACLWithPath& PreCheck();
  const setting::ACLWithPath& PreDirCheck();
  const setting::ACLWithPath& PostCheck();
  const std::vector<std::string>& IdleCommands();
  int TotalUsers();
  const std::vector<std::string>& Lslong();
  const std::vector<std::string>& HiddenFiles();
  const std::vector<std::string>& Noretrieve();
  const std::string& Tagline();
  const std::string& Email();
  const setting::Requests& Requests();
  const std::vector<int>& Lastonline();
  int EmptyNuke();
  const fs::Path& Nodupecheck();
  const setting::Creditcheck& Creditcheck();
  const setting::NukedirStyle& NukedirStyle();
  const std::vector<std::string>& Hideuser();
  const std::vector<std::string>& Privgroup();
  const std::vector<setting::MsgPath>& MsgPath();
  const std::vector<setting::ACLWithPath>& Privpath();
  int MaxSitecmdLines();
  const std::vector<setting::Cscript>& Cscript();
  const std::vector<setting::SiteCmd>& SiteCmd();

};

typedef std::tr1::shared_ptr<cfg::Config> ConfigPtr;

}


#endif 
