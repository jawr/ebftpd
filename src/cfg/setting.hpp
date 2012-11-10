#ifndef __CFG_SETTING_HPP
#define __CFG_SETTING_HPP

#include <string>
#include <vector>
#include <boost/function.hpp>                             
#include <boost/algorithm/string/join.hpp>                
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
#include <sys/types.h>
#include "acl/acl.hpp"
#include "fs/path.hpp"
#include "acl/passwdstrength.hpp"
#include "acl/ipstrength.hpp"
#include "main.hpp"

namespace cfg { namespace setting
{

class Setting
{
public:
  virtual ~Setting() {}
};

class Database : Setting
{
  std::string name;
  std::string address;
  int port;
public:
  Database() : name(programName), address("localhost"), port(27017) { }
  Database(const std::vector<std::string>& toks);
  
  const std::string& Name() const { return name; }
  const std::string& Address() const { return address; }
  int Port() const { return port; }
};

// generics
class Right : public Setting
{
  std::string path;
  // includes wildcards and possibley regex so can't be fs::Path path;
  acl::ACL acl;
public:
  Right() {}
  Right(std::vector<std::string> toks);
  const acl::ACL& ACL() const { return acl; }
  const std::string& Path() const { return path; }
};

class ACLInt : public Setting
{
  int arg;
  acl::ACL acl;
public:
  ACLInt() {}
  ACLInt(std::vector<std::string> toks);
  const acl::ACL& ACL() const { return acl; }
  int Int() const { return arg; }
};

// glftpd

class AsciiDownloads : public Setting
{
  off_t size;
  std::vector<std::string> masks;
  
public:
  AsciiDownloads() : size(-1) { }
  AsciiDownloads(const std::vector<std::string>& toks);
  bool Allowed(off_t size, const std::string& path) const;
};

class AsciiUploads : public Setting
{
  std::vector<std::string> masks;
  
public:
  AsciiUploads() { } 
  AsciiUploads(const std::vector<std::string>& toks);
  bool Allowed(const std::string& path) const;
};

class SecureIp : public Setting
{
  acl::IPStrength strength;
  acl::ACL acl;

public:
  SecureIp() : strength(2, false, true) { acl = acl::ACL::FromString("*"); } 
  
  SecureIp(std::vector<std::string> toks);
  const acl::IPStrength& Strength() const { return strength; }
  const acl::ACL& ACL() const { return acl; }
};

class SecurePass : public Setting
{
  acl::PasswdStrength strength;
  acl::ACL acl;
public:
  SecurePass(std::vector<std::string> toks);
  const acl::ACL& ACL() const { return acl; }
  const acl::PasswdStrength& Strength() const { return strength; }
};

class BouncerIp : public Setting
{
  std::vector<std::string> addrs;
public:
  BouncerIp() {}
  BouncerIp(const std::vector<std::string>& toks);
  const std::vector<std::string>& Addrs() const { return addrs; }
};

class SpeedLimit : public Setting
{
  fs::Path path;
  long dlLimit;
  long ulLimit;
  acl::ACL acl;
public:
  SpeedLimit() {}
  SpeedLimit(std::vector<std::string> toks);
  const fs::Path& Path() const { return path; }
  long DlLimit() const { return dlLimit; }
  long UlLimit() const { return ulLimit; }
  const acl::ACL& ACL() const { return acl; }
};

class SimXfers : public Setting
{
  int maxDownloads;
  int maxUploads;
public:
  SimXfers() {}
  SimXfers(std::vector<std::string> toks);
  int MaxDownloads() const { return maxDownloads; }
  int MaxUploads() const { return maxUploads; }
};

class PasvAddr : public Setting
{
  std::string addr;
  bool nat;
public:
  PasvAddr() {}
  PasvAddr(const std::vector<std::string>& toks);
  bool Nat() const { return nat; }
  const std::string& Addr() const { return addr; }
};

class PortRange
{
  int from;
  int to;
public:
  PortRange(int from, int to) : from(from), to(to) {}
  int From() const { return from; }
  int To() const { return to; }
};
 
// active/pasv ports
class Ports : public Setting
{
  std::vector<PortRange> ranges;
public:
  Ports() {}
  Ports(const std::vector<std::string>& toks);
  const std::vector<PortRange>& Ranges() const { return ranges; }
};
// end

class AllowFxp : public Setting
{
  bool downloads;
  bool uploads;
  bool logging;
  acl::ACL acl;
public:
  AllowFxp() {}
  AllowFxp(std::vector<std::string> toks);
  bool Downloads() const { return downloads; }
  bool Uploads() const { return uploads; }
  bool Logging() const { return logging; }
  const acl::ACL& ACL() const { return acl; }  
};

class Alias : public Setting
{
  std::string name;
  fs::Path path;
public:
  Alias() {}
  Alias(const std::vector<std::string>& toks);
  const std::string& Name() const { return name; }
  const fs::Path& Path() const { return path; }
};

class StatSection : public Setting
{
  std::string keyword;
  std::string path;
  bool seperateCredits;
public:
  StatSection() {}
  StatSection(const std::vector<std::string>& toks);
  const std::string& Keyword() const { return keyword; }
  const std::string& Path() const { return path; }
  bool SeperateCredits() const { return seperateCredits; }
};

class PathFilter : public Setting
{
  std::string messagePath;
  boost::regex regex;
  acl::ACL acl;
  
public:
  PathFilter() { }
  PathFilter(std::vector<std::string> toks);
  const std::string& MessagePath() const { return messagePath; }
  const boost::regex& Regex() const { return regex; }
  const acl::ACL& ACL() const { return acl; }
  
};

class MaxUsers : public Setting
{
  int users;
  int exemptUsers;
public:
  MaxUsers() : users(10), exemptUsers(5) {}
  MaxUsers(const std::vector<std::string>& toks);
  int Users() const { return users; }
  int ExemptUsers() const { return exemptUsers; }
};

class ShowTotals : public Setting
{
  int maxLines;
  std::vector<std::string> paths;
public:
  ShowTotals() {}
  ShowTotals(std::vector<std::string> toks);
  int MaxLines() const { return maxLines; }
};

class DupeCheck : public Setting
{
  int days;
  bool ignoreCase;
public:
  DupeCheck() {}
  DupeCheck(const std::vector<std::string>& toks);
  int Days() const { return days; }
  bool IgnoreCase() const { return ignoreCase; }
};

class Lslong : public Setting
{
  fs::Path bin;
  std::string options;
  int maxRecursion;
public:
  Lslong() {}
  Lslong(std::vector<std::string> toks);
  const fs::Path& Bin() const { return bin; }
  const std::string& Options() const { return options; }
  int MaxRecursion() const { return maxRecursion; }
};

class HiddenFiles : public Setting
{
  fs::Path path;
  std::vector<std::string> masks;
public:
  HiddenFiles() {}
  HiddenFiles(std::vector<std::string> toks);
  const fs::Path& Path() const { return path; }
  const std::vector<std::string>& Masks() const { return masks; }
};

class Requests : public Setting
{
  fs::Path path;
  int max;
public:
  Requests() : path("/ftp-data/misc/requests"), max(10) {}
  Requests(const std::vector<std::string>& toks);
  const fs::Path& Path() const { return path; }                              
  int Max() const { return max; }
};

class Lastonline : public Setting
{
  enum Type { ALL, TIMEOUT, ALL_WITH_ACTIVITY };
  Type type;
  int max;
public:
  Lastonline() : type(ALL), max(10) {}
  Lastonline(const std::vector<std::string>& toks);
  Type GetType() const { return type; } // anonymous enum?
  int Max() const { return max; }
};

class Creditcheck : public Setting
{
  fs::Path path;
  int ratio;
  acl::ACL acl;
public:
  Creditcheck() {}
  Creditcheck(std::vector<std::string> toks);
  const fs::Path& Path() const { return path; }
  int Ratio() const { return ratio; }
  const acl::ACL& ACL() const { return acl; }
};

class Creditloss : public Setting
{
  int multiplier;
  bool allowLeechers;
  fs::Path path;
  acl::ACL acl;
public:
  Creditloss() {}
  Creditloss(std::vector<std::string> toks);
  const acl::ACL& ACL() const { return acl; }
  int Multiplier() const { return multiplier; }
  bool AllowLeechers() const { return allowLeechers; }
  const fs::Path& Path() const { return path; }
};

class NukedirStyle : public Setting
{
  std::string format;
  enum Type { DELETE_ALL, DELETE_FILES, KEEP };
  Type type;
  int minBytes;
public:
  NukedirStyle() {}
  NukedirStyle(const std::vector<std::string>& toks);
  const std::string& Format() const { return format; }
  int MinBytes() const { return minBytes; }
  Type GetType() const { return type; } // anonymous enum?
};

class Privgroup : public Setting
{
  std::string group;
  std::string description;
public:
  Privgroup() : group("STAFF"), description("Staff Group") {}
  Privgroup(const std::vector<std::string>& toks);
  const std::string& Group() const { return group; }
  const std::string& Description() const { return description; }
};

class Msgpath : public Setting
{
  std::string path;
  fs::Path file;
  acl::ACL acl;
public:
  Msgpath() {}
  Msgpath(const std::vector<std::string>& toks);
  const std::string& Path() const { return path; }
  const acl::ACL& ACL() const { return acl; }
  const fs::Path& File() const { return file; }
};

class Privpath : public Setting
{
  fs::Path path; // no wildcards to avoid slowing down listing
  acl::ACL acl;
public:
  Privpath() {}
  Privpath(std::vector<std::string> toks);
  const fs::Path& Path() const { return path; }
  const acl::ACL& ACL() const { return acl; }
};

class SiteCmd : public Setting
{
  std::string command;
  enum Type { EXEC, TEXT, IS };
  Type type;
  std::string script;
  std::vector<std::string> arguments;
public:
  SiteCmd() {}
  SiteCmd(const std::vector<std::string>& toks);
  const std::string& Command() const { return command; }
  Type GetType() const { return type; } // anonymous enum?                 
  const std::vector<std::string>& Arguments() const { return arguments; }
  const std::string& Script() const { return script; }
};

class Cscript : public Setting
{
  std::string command;
  enum Type { PRE, POST };
  Type type;
  fs::Path script;
public:
  Cscript() {}
  Cscript(const std::vector<std::string>& toks);
  const std::string& Command() const { return command; }                       
  const fs::Path& Script() const { return script; }
  Type GetType() const { return type; } // anonymous enum?
};

class IdleTimeout : public Setting
{
  boost::posix_time::seconds maximum;
  boost::posix_time::seconds minimum;
  boost::posix_time::seconds timeout;
  
  static const boost::posix_time::seconds defaultMaximum;
  static const boost::posix_time::seconds defaultMinimum;
  static const boost::posix_time::seconds defaultTimeout;
  
public:
  IdleTimeout() :
    maximum(defaultMaximum), minimum(defaultMinimum),
    timeout(defaultTimeout) { }
    
  IdleTimeout(const std::vector<std::string>& toks);
    
  boost::posix_time::seconds Maximum() const { return maximum; }
  boost::posix_time::seconds Minimum() const { return minimum; }
  boost::posix_time::seconds Timeout() const { return timeout; }
};

// end namespace
}
}
#endif
