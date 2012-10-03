#ifndef __CFG_SETTING_HPP
#define __CFG_SETTING_HPP

#include <string>
#include <vector>
#include <boost/function.hpp>                                                   
#include <boost/bind.hpp>                                                       
#include <boost/algorithm/string/join.hpp>                                      
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include "acl/acl.hpp"
#include "fs/path.hpp"

namespace cfg { namespace setting
{
class Setting {
public:
  virtual ~Setting() {};
  virtual void Save(std::vector<std::string>& toks) = 0;
  void Save(std::string& toks) {
    std::vector<std::string> temp;
    boost::split(temp, toks, boost::is_any_of("\t "));
    Save(temp);                                                                 
  };
};

class AsciiDownloads : public Setting
{
  int size;
  std::vector<std::string> masks;
public:
  AsciiDownloads() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Shutdown : public Setting
{
  acl::ACL acl;
public:
  Shutdown() {};
  virtual void Save(std::vector<std::string>& toks);
};

class FreeSpace : public Setting
{
  int amount;
public:
  FreeSpace() {}
  virtual void Save(std::vector<std::string>& toks);
};

class UseDirSize : public Setting
{
  char unit;
  std::vector<fs::Path> paths;
public:
  UseDirSize() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Timezone : public Setting
{
  unsigned int hours;
public:
  Timezone() {};
  virtual void Save(std::vector<std::string>& toks);
};

class ColorMode : public Setting
{
  bool use;
public:
  ColorMode() {};
  virtual void Save(std::vector<std::string>& toks);
};

class SitenameLong : public Setting
{
  std::string name;
public:
  SitenameLong() {};
  virtual void Save(std::vector<std::string>& toks);
};

class SitenameShort : public Setting
{
  std::string name;
public:
  SitenameShort() {};
  virtual void Save(std::vector<std::string>& toks);
};

class LoginPrompt : public Setting
{
  std::string arg;
public:
  LoginPrompt() {};
  virtual void Save(std::vector<std::string>& toks);
};

class RootPath : public Setting
{
  fs::Path path;
public:
  RootPath() {};
  virtual void Save(std::vector<std::string>& toks);
};

class ReloadConfig : public Setting
{
  fs::Path path;
public:
  ReloadConfig() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Master : public Setting
{
  std::vector<std::string> users;
public:
  Master() {};
  virtual void Save(std::vector<std::string>& toks);
};

class SecureIp : public Setting
{
  int minFields;
  bool allowHostname;
  bool needIdent;
  acl::ACL acl;
public:
  SecureIp() {};
  virtual void Save(std::vector<std::string>& toks);
};

class SecurePass : public Setting
{
  std::string mask;
  acl::ACL acl;
public:
  SecurePass() {};
  virtual void Save(std::vector<std::string>& toks);
};

class DataPath : public Setting
{
  fs::Path path;
public:
  DataPath() {};
  virtual void Save(std::vector<std::string>& toks);
};

class PwdPath : public Setting
{
  fs::Path path;
public:
  PwdPath() {};
  virtual void Save(std::vector<std::string>& toks);
};

class GrpPath : public Setting
{
  fs::Path path;
public:
  GrpPath() {};
  virtual void Save(std::vector<std::string>& toks);
};

class BotscriptPath : public Setting
{
  fs::Path path;
public:
  BotscriptPath() {};
  virtual void Save(std::vector<std::string>& toks);
};

class BouncerIp : public Setting
{
  std::vector<std::string> addrs;
public:
  BouncerIp() {};
  virtual void Save(std::vector<std::string>& toks);
};

class SpeedLimit : public Setting
{
  fs::Path path;
  long dlLimit;
  long ulLimit;
  acl::ACL acl;
public:
  SpeedLimit() {};
  virtual void Save(std::vector<std::string>& toks);
};

class SimXfers : public Setting
{
  unsigned int maxDownloads;
  unsigned int maxUploads;
public:
  SimXfers() {};
  virtual void Save(std::vector<std::string>& toks);
};

class CalcCrc : public Setting
{
  std::vector<std::string> masks;
public:
  CalcCrc() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Xdupe : public Setting
{
  std::vector<std::string> masks;
public:
  Xdupe() {};
  virtual void Save(std::vector<std::string>& toks);
};

class MmapAmount : public Setting
{
  int size;
public:
  MmapAmount() {};
  virtual void Save(std::vector<std::string>& toks);
};

class DlSendfile : public Setting
{
  int size;
public:
  DlSendfile() {};
  virtual void Save(std::vector<std::string>& toks);
};

class UlBufferedForce : public Setting
{
  int size;
public:
  UlBufferedForce() {};
  virtual void Save(std::vector<std::string>& toks);
};

class MinHomedir : public Setting
{
  fs::Path path;
public:
  MinHomedir() {};
  virtual void Save(std::vector<std::string>& toks);
};

class ValidIp : public Setting
{
  std::vector<std::string> ips;
public:
  ValidIp() {};
  virtual void Save(std::vector<std::string>& toks);
};

class ActiveAddr : public Setting
{
  std::string addr;
public:
  ActiveAddr() {};
  virtual void Save(std::vector<std::string>& toks);
};

class PasvAddr : public Setting
{
  std::string addr;
  bool nat;
public:
  PasvAddr() {};
  virtual void Save(std::vector<std::string>& toks);
};

class PortRange
{
  int from;
  int to;
public:
  PortRange(int from) : from(from), to(from) {};
  PortRange(int from, int to) : from(from), to(to) {};
};
 
// active/pasv ports
class Ports : public Setting
{
  std::vector<PortRange> ranges;
public:
  Ports() {};
  virtual void Save(std::vector<std::string>& toks);
};
// end

class AllowFxp : public Setting
{
  bool downloads;
  bool uploads;
  bool logging;
  acl::ACL acl;
public:
  AllowFxp() {};
  virtual void Save(std::vector<std::string>& toks);
};

class WelcomeMsg : public Setting
{
  fs::Path path;
  acl::ACL acl;
public:
  WelcomeMsg() {};
  virtual void Save(std::vector<std::string>& toks);
};

class GoodbyeMsg : public Setting
{
  fs::Path path;
  acl::ACL acl;
public:
  GoodbyeMsg() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Newsfile : public Setting
{
  fs::Path path;
  acl::ACL acl;
public:
  Newsfile() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Banner : public Setting
{
  fs::Path path;
public:
  Banner() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Alias : public Setting
{
  std::string alias;
  fs::Path path;
public:
  Alias() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Cdpath : public Setting
{
  fs::Path path;
public:
  Cdpath() {};
  virtual void Save(std::vector<std::string>& toks);
};

class IgnoreType : public Setting
{
  std::vector<std::string> masks;
public:
  IgnoreType() {};
  virtual void Save(std::vector<std::string>& toks);
};

// rights sections

class Right : public Setting
{
  std::string path;
  // includes wildcards and possibley regex so can't be fs::Path path;
  acl::ACL acl;
public:
  Right() {};
  virtual void Save(std::vector<std::string>& toks);
};

// end rights

class StatSection : public Setting
{
  std::string keyword;
  std::string path;
  bool seperateCredits;
public:
  StatSection() {};
  virtual void Save(std::vector<std::string>& toks);
};

class PathFilter : public Setting
{
  std::string group;
  std::string path;
  std::vector<std::string> filters;
public:
  PathFilter() {};
  virtual void Save(std::vector<std::string>& toks);
};

class MaxUsers : public Setting
{
  int maxUsers;
  int maxExemptUsers;
public:
  MaxUsers() {};
  virtual void Save(std::vector<std::string>& toks);
};

class MaxUstats : public Setting
{
  int maxResults;
  acl::ACL acl;
public:
  MaxUstats() {};
  virtual void Save(std::vector<std::string>& toks);
};

class MaxGstats : public Setting
{
  int maxResults;
  acl::ACL acl;
public:
  MaxGstats() {};
  virtual void Save(std::vector<std::string>& toks);
};

class BannedUsers : public Setting
{
  std::vector<std::string> users;
public:
  BannedUsers() {};
  virtual void Save(std::vector<std::string>& toks);
};

class ShowDiz : public Setting
{
  std::string filename;
  acl::ACL acl;
public:
  ShowDiz() {};
  virtual void Save(std::vector<std::string>& toks);
};

class ShowTotals : public Setting
{
  int maxLines;
  std::vector<std::string> paths;
public:
  ShowTotals() {};
  virtual void Save(std::vector<std::string>& toks);
};

class DlIncomplete : public Setting
{
  bool enabled;
public:
  DlIncomplete() {};
  virtual void Save(std::vector<std::string>& toks);
};

class FileDlCount : public Setting
{
  bool enabled;
public:
  FileDlCount() {};
  virtual void Save(std::vector<std::string>& toks);
};

class DupeCheck : public Setting
{
  int days;
  bool ignoreCase;
public:
  DupeCheck() {};
  virtual void Save(std::vector<std::string>& toks);
};

// in built scripts
// i.e. pre, pre_dir, post
class Script : public Setting
{
  fs::Path script;
  std::vector<std::string> masks;
public:
  Script() {};
  virtual void Save(std::vector<std::string>& toks);
};

class IdleCommands : public Setting
{
  std::vector<std::string> commands;
public:
  IdleCommands() {};
  virtual void Save(std::vector<std::string>& toks);
};

class TotalUsers : public Setting
{
  int limit;
public:
  TotalUsers() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Lslong : public Setting
{
  fs::Path bin;
  std::vector<std::string> options;
  int maxRecursion;
public:
  Lslong() {};
  virtual void Save(std::vector<std::string>& toks);
};

class HiddenFiles : public Setting
{
  fs::Path path;
  std::vector<std::string> masks;
public:
  HiddenFiles() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Noretrieve : public Setting
{
  std::vector<std::string> masks;
public:
  Noretrieve() {};
  virtual void Save(std::vector<std::string>& toks);
};

// do not implement
// class NameRule : public Setting
// {
// public:
//   NameRule() {};
// };
// end
  
class Tagline : public Setting
{
  std::string tagline;
public:
  Tagline() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Email : public Setting
{
  std::string email;
public: 
  Email() {};
  virtual void Save(std::vector<std::string>& toks);
};

class MultiplierMax : public Setting
{
  int max;
public:
  MultiplierMax() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Oneliners : public Setting
{
  int max;
public:
  Oneliners() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Requests : public Setting
{
  fs::Path path;
  int max;
public:
  Requests() {};
  virtual void Save(std::vector<std::string>& toks);
};

// lastonline
namespace lastonline
{
enum Type { ALL, TIMEOUT, ALL_WITH_ACTIVITY };
} 
class Lastonline : public Setting
{
  lastonline::Type type;
  int max;
public:
  Lastonline() {};
  virtual void Save(std::vector<std::string>& toks);
};
// end lastonline

class EmptyNuke : public Setting
{
  int amount;
public:
  EmptyNuke() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Nodupecheck : public Setting
{
  fs::Path path;
public:
  Nodupecheck() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Creditcheck : public Setting
{
  fs::Path path;
  int ratio;
  acl::ACL acl;
public:
  Creditcheck() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Creditloss : public Setting
{
  int multiplier;
  bool allowLeechers;
  fs::Path path;
  acl::ACL acl;
public:
  Creditloss() {};
  virtual void Save(std::vector<std::string>& toks);
};

// nukedir_style
namespace nukedirstyle {
  enum Type { DELETE_ALL, DELETE_FILES, KEEP };
}

class NukedirStyle : public Setting
{
  std::string format;
  nukedirstyle::Type type;
  int minBytes;
public:
  NukedirStyle() {};
  virtual void Save(std::vector<std::string>& toks);
};
// end

class Hideuser : public Setting
{
  acl::ACL acl;
public:
  Hideuser() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Privgroup : public Setting
{
  std::string group;
  std::string description;
public:
  Privgroup() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Msgpath : public Setting
{
  std::string path;
  fs::Path file;
  acl::ACL acl;
public:
  Msgpath() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Privpath : public Setting
{
  fs::Path path; // no wildcards to avoid slowing down listing
  acl::ACL acl;
public:
  Privpath() {};
  virtual void Save(std::vector<std::string>& toks);
};

// might want to register these in a seperate factory
namespace sitecmd
{
  enum Type { EXEC, TEXT, IS };
}
class SiteCmd : public Setting
{
  std::string command;
  sitecmd::Type type;
  std::string script;
  std::vector<std::string> arguments;
public:
  SiteCmd() {};
  virtual void Save(std::vector<std::string>& toks);
};

// end

class MaxSitecmdLines : public Setting
{
  int max;
public:
  MaxSitecmdLines() {};
  virtual void Save(std::vector<std::string>& toks);
};

// custom scripts
namespace cscript
{
  enum Type { PRE, POST };
}
class Cscript : public Setting
{
  std::string command;
  cscript::Type type;
  fs::Path script;
public:
  Cscript() {};
  virtual void Save(std::vector<std::string>& toks);
};

// end namespace
}
}
#endif
