//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __CFG_CONFIG_CPP
#define __CFG_CONFIG_CPP

#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <unordered_set>
#include <map>
#include <boost/optional.hpp>
#include <sys/types.h>
#include "cfg/setting.hpp"
#include "cfg/section.hpp"
#include "util/enumstrings.hpp"

namespace cfg
{

enum class EPSVFxp { Allow, Deny, Force };
enum class LogAddresses { Never, Errors, Always };

class Config;

typedef std::shared_ptr<cfg::Config> ConfigPtr;

class Config
{ 
  int version;
  bool tool;
 
  void ParseGlobal(const std::string& opt, std::vector<std::string>& toks);
  void ParseSection(const std::string& opt, std::vector<std::string>& toks);
  void Parse(std::string line);
  
  bool CheckSetting(const std::string& name);
  void SanityCheck();
  void SetDefaults(const std::string& opt); 

  std::unordered_map<std::string, int> settingsCache;
  Section* currentSection;

  // containers
  std::string sitepath;
  std::string pidfile;
  std::string tlsCertificate;
  std::string tlsCiphers;
  int port;
  // glftpd
  ::cfg::AsciiDownloads asciiDownloads;
  ::cfg::AsciiUploads asciiUploads;
  long long freeSpace;
  std::string sitenameLong;
  std::string sitenameShort;
  std::string loginPrompt;
  std::vector<std::string> master;
  std::vector< ::cfg::SecureIp> secureIp;
  std::vector< ::cfg::SecurePass> securePass;
  std::string datapath;
  std::vector<std::string> bouncerIp;
  bool bouncerOnly;
  std::vector<SpeedLimit> maximumSpeed;
  std::vector<SpeedLimit> minimumSpeed;
  ::cfg::SimXfers simXfers;
  std::vector<std::string> calcCrc;
  std::vector<std::string> xdupe;
  std::vector<std::string> validIp;
  std::vector<std::string> activeAddr;
  std::vector<std::string> pasvAddr;
  Ports activePorts;
  Ports pasvPorts;
  std::vector< ::cfg::AllowFxp> allowFxp;
  std::vector< ::cfg::Right> welcomeMsg;
  std::vector< ::cfg::Right> goodbyeMsg;
  std::string banner;
  std::vector< ::cfg::Alias> alias;
  std::vector<std::string> cdpath;
  
  Log securityLog;
  Log databaseLog;
  Log eventLog;
  Log errorLog;
  Log debugLog;
  Log siteopLog;
  ::cfg::TransferLog transferLog;
  
  // ind rights
  std::vector< ::cfg::Right> delete_; // delete is reserved
  std::vector< ::cfg::Right> deleteown;
  std::vector< ::cfg::Right> overwrite;
  std::vector< ::cfg::Right> overwriteown;
  std::vector< ::cfg::Right> resume;
  std::vector< ::cfg::Right> resumeown;
  std::vector< ::cfg::Right> rename;
  std::vector< ::cfg::Right> renameown;
  std::vector< ::cfg::Right> move;
  std::vector< ::cfg::Right> moveown;
  std::vector< ::cfg::Right> makedir;
  std::vector< ::cfg::Right> upload;
  std::vector< ::cfg::Right> download;
  std::vector< ::cfg::Right> downloadown;
  std::vector< ::cfg::Right> nuke;
  std::vector< ::cfg::Right> hideinwho;
  std::vector< ::cfg::Right> freefile;
  std::vector< ::cfg::Right> nostats;
  std::vector< ::cfg::Right> hideowner;
  std::vector< ::cfg::Right> modify;
  std::vector< ::cfg::Right> modifyown;

  std::vector<std::string> eventpath;
  std::vector<std::string> dupepath;
  std::vector<std::string> indexpath;

  // end rights
  std::vector< ::cfg::PathFilter> pathFilter;
  ::cfg::MaxUsers maxUsers;
  std::vector<ACLInt> maxUstats;
  std::vector<ACLInt> maxGstats;
  std::vector<std::string> bannedUsers;
  std::vector< ::cfg::Right> showDiz;
  bool dlIncomplete;
  std::vector< ::cfg::Cscript> cscript;
  std::vector<std::string> idleCommands;
  int totalUsers;
  ::cfg::Lslong lslong;
  std::vector< ::cfg::HiddenFiles> hiddenFiles;
  std::vector<std::string> noretrieve;
  ::cfg::NukeMax nukeMax;
  std::vector< ::cfg::Creditcheck> creditcheck;
  std::vector< ::cfg::Creditloss> creditloss;
  ::cfg::NukeStyle nukedirStyle;
  std::vector< ::cfg::Msgpath> msgpath;
  std::vector< ::cfg::Privpath> privpath;
  std::vector< ::cfg::SiteCmd> siteCmd;
  int maxSitecmdLines;
  ::cfg::IdleTimeout idleTimeout;
  ::cfg::Database database;
  std::vector<CheckScript> preCheck;
  std::vector<CheckScript> preDirCheck;
  std::vector<CheckScript> postCheck;
  std::unordered_map<std::string, acl::ACL> commandACLs;  
  std::map<std::string, Section> sections;
  ::cfg::EPSVFxp epsvFxp;
  int maximumRatio;
  int dirSizeDepth;
  bool asyncCRC;
  bool identLookup;
  bool dnsLookup;
  ::cfg::LogAddresses logAddresses;
  mode_t umask;
  int logLines;
  ssize_t dataBufferSize;
  std::string natAddr;
  
  acl::ACL tlsControl;
  acl::ACL tlsListing;
  acl::ACL tlsData;
  acl::ACL tlsFxp;
  
  static std::unordered_set<std::string> aclKeywords;
  static int latestVersion;
  static const std::vector<std::string> requiredSettings;
  static const std::string configFile;
  static const std::vector<std::string> configSearch;
  static std::string lastConfigPath;
  static boost::optional< ::cfg::MaxUsers> maxOnline;
  
  Config(const Config&) = default;
  Config& operator=(const Config&) = default;
  
  static void ParameterCheck(const std::string& opt,
                             const std::vector<std::string>& toks, 
                             int minimum, int maximum);
  static void ParameterCheck(const std::string& opt,
                             const std::vector<std::string>& toks, 
                             int minimum)
  { ParameterCheck(opt, toks, minimum, minimum); }


public:
  Config(const std::string& configPath, bool tool = false);

  int Version() const { return version; }

  const ::cfg::Database& Database() const { return database; }
  const std::string& Sitepath() const { return sitepath; }
  const std::string& Pidfile() const { return pidfile; }
  const std::string& TlsCertificate() const { return tlsCertificate; }
  const std::string& TlsCiphers() const { return tlsCiphers; }
  int Port() const { return port; }
  const ::cfg::AsciiDownloads& AsciiDownloads() const { return asciiDownloads; } 
  const ::cfg::AsciiUploads& AsciiUploads() const { return asciiUploads; } 
  long long FreeSpace() const { return freeSpace; }
  const std::string& SitenameLong() const { return sitenameLong; }
  const std::string& SitenameShort() const { return sitenameShort; }
  const std::string& LoginPrompt() const { return loginPrompt; }
  const std::vector<std::string>& Master() const { return master; }  
  bool IsMaster(const std::string& login) const
  {
    return std::find(master.begin(), master.end(), login) != master.end();
  }
  const std::vector< ::cfg::SecureIp>& SecureIp() const { return secureIp; }
  const std::vector< ::cfg::SecurePass>& SecurePass() const { return securePass; }
  const std::string& Datapath() const { return datapath; }
  //const std::vector<std::string>& BouncerIp() const { return bouncerIp; }
  bool IsBouncer(const std::string& ip) const;
  bool BouncerOnly() const { return bouncerOnly; }
  const std::vector<SpeedLimit>& MaximumSpeed() const { return maximumSpeed; }
  const std::vector<SpeedLimit>& MinimumSpeed() const { return minimumSpeed; }
  const ::cfg::SimXfers& SimXfers() const { return simXfers; }
  const std::vector<std::string>& CalcCrc() const { return calcCrc; }
  const std::vector<std::string>& Xdupe() const { return xdupe; }
  const std::vector<std::string>& ValidIp() const { return validIp; }
  const std::vector<std::string>& ActiveAddr() const { return activeAddr; }
  const std::vector<std::string>& PasvAddr() const { return pasvAddr; }
  const Ports& ActivePorts() const { return activePorts; }
  const Ports& PasvPorts() const { return pasvPorts; }
  const std::vector< ::cfg::AllowFxp>& AllowFxp() const { return allowFxp; }
  const std::vector< ::cfg::Right>& WelcomeMsg() const { return welcomeMsg; }
  const std::vector< ::cfg::Right>& GoodbyeMsg() const { return goodbyeMsg; }
  const std::string& Banner() const { return banner; }
  const std::vector< ::cfg::Alias>& Alias() const { return alias; }
  const std::vector<std::string>& Cdpath() const { return cdpath; }
  
  const Log SecurityLog() const { return securityLog; }
  const Log DatabaseLog() const { return databaseLog; }
  const Log EventLog() const { return eventLog; }
  const Log ErrorLog() const { return errorLog; }
  const Log DebugLog() const { return debugLog; }
  const Log SiteopLog() const { return siteopLog; }
  const ::cfg::TransferLog TransferLog() const { return transferLog; }

  // rights section
  const std::vector< ::cfg::Right>& Delete() const { return delete_; } 
  const std::vector< ::cfg::Right>& Deleteown() const { return deleteown; } 
  const std::vector< ::cfg::Right>& Overwrite() const { return overwrite; } 
  const std::vector< ::cfg::Right>& Overwriteown() const { return overwriteown; } 
  const std::vector< ::cfg::Right>& Resume() const { return resume; } 
  const std::vector< ::cfg::Right>& Resumeown() const { return resumeown; } 
  const std::vector< ::cfg::Right>& Rename() const { return rename; } 
  const std::vector< ::cfg::Right>& Renameown() const { return renameown; } 
  const std::vector< ::cfg::Right>& Move() const { return move; } 
  const std::vector< ::cfg::Right>& Moveown() const { return moveown; } 
  const std::vector< ::cfg::Right>& Makedir() const { return makedir; } 
  const std::vector< ::cfg::Right>& Upload() const { return upload; } 
  const std::vector< ::cfg::Right>& Download() const { return download; } 
  const std::vector< ::cfg::Right>& Downloadown() const { return downloadown; } 
  const std::vector< ::cfg::Right>& Modify() const { return modify; } 
  const std::vector< ::cfg::Right>& Modifyown() const { return modifyown; } 
  const std::vector< ::cfg::Right>& Nuke() const { return nuke; } 
  const std::vector< ::cfg::Right>& Hideinwho() const { return hideinwho; } 
  const std::vector< ::cfg::Right>& Freefile() const { return freefile; } 
  const std::vector< ::cfg::Right>& Nostats() const { return nostats; } 
  const std::vector< ::cfg::Right>& Hideowner() const { return hideowner; } 

  bool IsEventLogged(const std::string& path) const;
  bool IsDupeLogged(const std::string& path) const;
  bool IsIndexed(const std::string& path) const;
  const std::vector<std::string>& Indexed() const { return indexpath; }

  const std::vector< ::cfg::PathFilter>& PathFilter() const { return pathFilter; }
  const ::cfg::MaxUsers& MaxUsers() const { return maxUsers; }
  const std::vector<ACLInt>& MaxUstats() const { return maxUstats; }
  const std::vector<ACLInt>& MaxGstats() const { return maxGstats; }
  const std::vector<std::string>& BannedUsers() const { return bannedUsers; }
  const std::vector< ::cfg::Right>& ShowDiz() const { return showDiz; }
  bool DlIncomplete() const { return dlIncomplete; }
  const std::vector< ::cfg::Cscript>& Cscript() const { return cscript; }
  const std::vector<std::string>& IdleCommands() const { return idleCommands; }
  int TotalUsers() const { return totalUsers; }
  const ::cfg::Lslong& Lslong() const { return lslong; }
  const std::vector< ::cfg::HiddenFiles>& HiddenFiles() const { return hiddenFiles; }
  const std::vector<std::string>& Noretrieve() const { return noretrieve; }
  const ::cfg::NukeMax& NukeMax() const { return nukeMax; }
  const std::vector< ::cfg::Creditcheck>& Creditcheck() const { return creditcheck; }
  const std::vector< ::cfg::Creditloss>& Creditloss() const { return creditloss; }
  const ::cfg::NukeStyle& NukeStyle() const { return nukedirStyle; }
  const std::vector< ::cfg::Msgpath>& Msgpath() const { return msgpath; }
  const std::vector< ::cfg::Privpath>& Privpath() const { return privpath; }
  const std::vector< ::cfg::SiteCmd>& SiteCmd() const { return siteCmd; }
  int MaxSitecmdLines() const { return maxSitecmdLines; }
  const ::cfg::IdleTimeout& IdleTimeout() const { return idleTimeout; }
  const std::vector<CheckScript>& PreCheck() const { return preCheck; }
  const std::vector<CheckScript>& PreDirCheck() const { return preDirCheck; }
  const std::vector<CheckScript>& PostCheck() const { return postCheck; }  
  const std::map<std::string, Section>& Sections() const { return sections; }
  boost::optional<const Section&> SectionMatch(std::string path, bool isDir = false) const;
  ::cfg::EPSVFxp EPSVFxp() const { return epsvFxp; }
  int MaximumRatio() const { return maximumRatio; }
  const acl::ACL& TLSControl() const { return tlsControl; }
  const acl::ACL& TLSListing() const { return tlsListing; }
  const acl::ACL& TLSData() const { return tlsData; }
  const acl::ACL& TLSFxp() const { return tlsFxp; }
  int DirSizeDepth() const { return dirSizeDepth; }
  bool AsyncCRC() const { return asyncCRC; }
  bool IdentLookup() const { return identLookup; }
  bool DNSLookup() const { return dnsLookup; }
  ::cfg::LogAddresses LogAddresses() const { return logAddresses; }
  mode_t Umask() const { return umask; }
  int LogLines() const { return logLines; }
  size_t DataBufferSize() const { return dataBufferSize; }
  const std::string& NATAddr() const { return natAddr; }
  
  const acl::ACL& CommandACL(const std::string& keyword) const
  { return commandACLs.at(keyword); }
  
  static void PopulateACLKeywords(const std::unordered_set<std::string>& keywords)
  { aclKeywords.insert(keywords.begin(), keywords.end()); }
  
  static ConfigPtr Load(std::string configPath = lastConfigPath, bool tool = false);
  
  static const ::cfg::MaxUsers& MaxOnline() { return *maxOnline; }
  
  friend void UpdateLocal();
};

}

#endif 
