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
  void SetDefaults();

  // containers
  boost::unordered_map<std::string, std::vector<fs::Path> > MapPath;
  boost::unordered_map<std::string, std::vector<ACL> > MapACL;
  boost::unordered_map<std::string, std::vector<IntWithArguments> > MapIntWithArguments;
  boost::unordered_map<std::string, std::vector<std::string> > MapString;
  boost::unordered_map<std::string, std::vector<std::vector<std::string> > > MapVectorString;
  boost::unordered_map<std::string, std::vector<int> > MapInt;
  boost::unordered_map<std::string, std::vector<std::vector<int> > > MapVectorInt;
  boost::unordered_map<std::string, std::vector<bool> > MapBool; 
  boost::unordered_map<std::string, std::vector<SecureIpOpt> > MapSecureIp;
  boost::unordered_map<std::string, std::vector<SpeedLimitOpt> > MapSpeedLimit;
  boost::unordered_map<std::string, std::vector<PasvAddrOpt> > MapPasvAddr;
  boost::unordered_map<std::string, std::vector<AllowFxpOpt> > MapAllowFxp;
  boost::unordered_map<std::string, std::vector<ACLWithPath> > MapACLWithPath;
  boost::unordered_map<std::string, std::vector<PathWithArgument> > MapPathWithArgument;
  boost::unordered_map<std::string, std::vector<ACLWithArgument> > MapACLWithArgument;
  boost::unordered_map<std::string, std::vector<StatSectionOpt> > MapStatSection;
  boost::unordered_map<std::string, std::vector<PathFilterOpt> > MapPathFilter;
  boost::unordered_map<std::string, std::vector<ACLWithInt> > MapACLWithInt;
  boost::unordered_map<std::string, std::vector<IntWithBool> > MapIntWithBool;
  boost::unordered_map<std::string, std::vector<RequestsOpt> > MapRequests; 
  boost::unordered_map<std::string, std::vector<CreditcheckOpt> > MapCreditcheck;
  boost::unordered_map<std::string, std::vector<CreditlossOpt> > MapCreditloss;
  boost::unordered_map<std::string, std::vector<NukedirStyleOpt> > MapNukedirStyle;
  boost::unordered_map<std::string, std::vector<MsgPathOpt> > MapMsgPath;

  // site commands
  boost::unordered_map<std::string, std::vector<SiteCmdOpt> > MapSiteCmd;

  // custom commands
  boost::unordered_map<std::string, std::vector<CscriptOpt> > MapCscript;


  template <typename T> void InsertSetting(
    boost::unordered_map<std::string, std::vector<T> >& map, T set, const std::string& opt)
  {
    if (map.find(opt) == map.end())
    {
      std::vector<T> vec;
      vec.push_back(set);
      map.insert(std::make_pair(opt, vec));
    }
    else
      map.at(opt).push_back(set);
  };

public:
  Config(const std::string& configFile);
  ~Config() {};  

  int Version() const { return version; };

  // getters
  // follows the following syntax:
  //  userrejectsecure -> Userrejectsecure
  //  ascii_downloads  -> AsciiDownloads 
  const fs::Path& DSACertFile() const { return MapPath.at("dsa_cert_file").back(); };
  const ACL& Userrejectsecure() const { return MapACL.at("userrejectsecure").back(); };
  const ACL& Userrejectinsecure() const { return MapACL.at("userrejectinsecure").back(); };
  const ACL& Denydiruncrypted() const { return MapACL.at("denydiruncrypted").back(); };
  const ACL& Denydatauncrypted() const { return MapACL.at("denydatauncrypted").back(); };
  const IntWithArguments& AsciiDownloads() const { return MapIntWithArguments.at("ascii_downloads").back(); };
  const ACL& Shutdown() const { return MapACL.at("shutdown").back(); };
  int FreeSpace() const { return MapInt.at("free_space").back(); };
  const std::vector<std::string>& UseDirSize() const { return MapVectorString.at("use_dir_size").back(); };
  int Timezone() const { return MapInt.at("timezone").back(); };
  bool ColorMode() const{ return MapBool.at("color_mode").back(); };
  const std::string& SitenameLong() const { return MapString.at("sitename_long").back(); };
  const std::string& SitenameShort() const { return MapString.at("sitename_short").back(); };
  const std::string& LoginPrompt() const { return MapString.at("login_prompt").back(); };
  const fs::Path& Rootpath() const { return MapPath.at("rootpath").back(); };
  const fs::Path& Datapath() const { return MapPath.at("datapath").back(); };
  const fs::Path& ReloadConfig() const { return MapPath.at("reload_config").back(); }; // needed?
  const std::vector<std::string>& Master() const { return MapVectorString.at("master").back(); };
  const std::vector<SecureIpOpt>& SecureIp() const { return MapSecureIp.at("secure_ip"); };
  const std::vector<ACLWithPath>& SecurePass() const { return MapACLWithPath.at("secure_pass"); };
  const fs::Path& PwdPath() const { return MapPath.at("pwd_path").back(); };
  const fs::Path& GrpPath() const { return MapPath.at("grp_path").back(); };
  const fs::Path& BotscriptPath() const { return MapPath.at("botscript_path").back(); };
  const std::vector<std::string>& BouncerIp() const { return MapVectorString.at("bouncer_ip").back(); };
  const std::vector<SpeedLimitOpt>& SpeedLimit() const { return MapSpeedLimit.at("speed_limit"); };
  const std::vector<int>& SimXfers() const { return MapVectorInt.at("sim_xfers").back(); };
  const fs::Path& CalcCrc() const { return MapPath.at("calc_crc").back(); };
  const std::vector<std::string>& Xdupe() const { return MapVectorString.at("xdupe").back(); };
  int MmapAmount() const { return MapInt.at("mmap_amount").back(); };
  int DlSendfile() const { return MapInt.at("dl_sendfile").back(); };
  int UlBufferedForce() const { return MapInt.at("ul_buffered_force").back(); };
  const fs::Path& MinHomedir() const { return MapPath.at("min_homedir").back(); };
  const std::vector<std::vector<std::string> >& ValidIp() const { return MapVectorString.at("valid_ip"); };
  const std::vector<std::vector<std::string> >& ActiveAddr() const { return MapVectorString.at("active_addr"); };
  const std::vector<PasvAddrOpt>& PasvAddr() const { return MapPasvAddr.at("pasv_addr"); };
  const std::vector<std::vector<std::string> >& PasvPorts() const { return MapVectorString.at("pasv_ports"); };
  const std::vector<std::vector<std::string> >& ActivePorts() const { return MapVectorString.at("active_ports"); };
  const AllowFxpOpt& AllowFxp() const { return MapAllowFxp.at("allow_fxp").back(); };
  const std::vector<ACLWithPath>& WelcomeMsg() const { return MapACLWithPath.at("welcome_msg"); };
  const std::vector<ACLWithPath>& GoodbyeMsg() const { return MapACLWithPath.at("goodbye_msg"); };
  const std::vector<ACLWithPath>& Newsfile() const { return MapACLWithPath.at("newsfile"); };
  const std::vector<fs::Path>& Banner() const { return MapPath.at("banner"); };
  const std::vector<PathWithArgument>& Alias() const { return MapPathWithArgument.at("alias"); };
  const std::vector<std::string>& Cdpath() const { return MapString.at("cdpath"); };
  const std::vector<std::vector<std::string> >& IgnoreType() const { return MapVectorString.at("ignore_type"); };
  const std::vector<ACLWithPath>& Delete() const { return MapACLWithPath.at("delete"); };   
  const std::vector<ACLWithPath>& Deleteown() const { return MapACLWithPath.at("deleteown"); };   
  const std::vector<ACLWithPath>& Overwrite() const { return MapACLWithPath.at("overwrite"); };   
  const std::vector<ACLWithPath>& Resume() const { return MapACLWithPath.at("resume"); };   
  const std::vector<ACLWithPath>& Rename() const { return MapACLWithPath.at("rename"); };   
  const std::vector<ACLWithPath>& Renameown() const { return MapACLWithPath.at("renameown"); };   
  const std::vector<ACLWithPath>& Filemove() const { return MapACLWithPath.at("filemove"); };   
  const std::vector<ACLWithPath>& Makedir() const { return MapACLWithPath.at("makedir"); };   
  const std::vector<ACLWithPath>& Upload() const { return MapACLWithPath.at("upload"); };
  const std::vector<ACLWithPath>& Download() const { return MapACLWithPath.at("download"); };   
  const std::vector<ACLWithPath>& Nuke() const { return MapACLWithPath.at("nuke"); };   
  const std::vector<ACLWithPath>& Dirlog() const { return MapACLWithPath.at("dirlog"); };   
  const std::vector<ACLWithPath>& Hideinwho() const { return MapACLWithPath.at("hideinwho"); };   
  const std::vector<ACLWithPath>& Nostats() const { return MapACLWithPath.at("nostats"); };   
  const std::vector<ACLWithArgument>& Freefile() const { return MapACLWithArgument.at("freefile"); };
  const std::vector<StatSectionOpt>& StatSection() const { return MapStatSection.at("stat_section"); };
  const std::vector<PathFilterOpt>& PathFilter() const { return MapPathFilter.at("path_filter"); };
  const std::vector<int>& MaxUsers() const { return MapVectorInt.at("max_users").back(); };
  const ACLWithInt& MaxUstats() const { return MapACLWithInt.at("max_ustats").back(); }; 
  const ACLWithInt& MaxGstats() const { return MapACLWithInt.at("mac_gstats").back(); }; 
  const std::vector<std::string>& BannedUsers() const { return MapVectorString.at("banned_users").back(); };
  const ACLWithPath& ShowDiz() const { return MapACLWithPath.at("show_diz").back(); };
  const IntWithArguments& ShowTotals() const { return MapIntWithArguments.at("show_total").back(); };
  bool DlIncomplete() const { return MapBool.at("dl_incomplete").back(); };
  bool FileDlCount() const { return MapBool.at("file_dl_count").back(); };
  const IntWithBool& DupeCheck() const { return MapIntWithBool.at("dupe_check").back(); };
  const ACLWithPath& PreCheck() const { return MapACLWithPath.at("pre_check").back(); };
  const ACLWithPath& PreDirCheck() const { return MapACLWithPath.at("pre_dir_check").back(); };
  const ACLWithPath& PostCheck() const { return MapACLWithPath.at("post_check").back(); };
  const std::vector<std::string>& IdleCommands() const { return MapVectorString.at("idle_commands").back(); };
  int TotalUsers() const { return MapInt.at("total_users").back(); };
  const std::vector<std::string>& Lslong() const { return MapVectorString.at("ls_long").back(); };
  const std::vector<std::string>& HiddenFiles() const { return MapVectorString.at("hidden_files").back(); };
  const std::vector<std::string>& Noretrieve() const { return MapVectorString.at("noretrieve").back(); };
  const std::string& Tagline() const { return MapString.at("tagline").back(); };
  const std::string& Email() const { return MapString.at("email").back(); };
  const RequestsOpt& Requests() const { return MapRequests.at("requests").back(); };
  const std::vector<int>& Lastonline() const { return MapVectorInt.at("lastonline").back(); };
  int EmptyNuke() const { return MapInt.at("empty_nuke").back(); };
  const fs::Path& Nodupecheck() const { return MapPath.at("nodupecheck").back(); };
  const std::vector<CreditcheckOpt>& Creditcheck() const { return MapCreditcheck.at("creditcheck"); };
  const NukedirStyleOpt& NukedirStyle() const { return MapNukedirStyle.at("nukedir_style").back(); };
  const ACL& Hideuser() const { return MapACL.at("hideuser").back(); };
  const std::vector<std::vector<std::string> >& Privgroup() const { return MapVectorString.at("privgroup"); };
  const std::vector<MsgPathOpt>& MsgPath() const { return MapMsgPath.at("msg_path"); };
  const std::vector<ACLWithPath>& Privpath() const { return MapACLWithPath.at("privpath"); };
  int MaxSitecmdLines() const { return MapInt.at("max_sitecmd_lines").back(); };
  const std::vector<CscriptOpt>& Cscript() const { return MapCscript.at("cscript"); };
  const std::vector<SiteCmdOpt>& SiteCmd() const { return MapSiteCmd.at("site_cmd"); };
  int Oneliners() const { return MapInt.at("oneliners").back(); };
  int MultiplierMax() const { return MapInt.at("MultiplierMax").back(); };

};

typedef std::tr1::shared_ptr<cfg::Config> ConfigPtr;

}


#endif 
