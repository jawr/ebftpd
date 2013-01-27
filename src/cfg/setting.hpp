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
  std::string login;
  std::string password;
  
public:
  Database() : name(programName), address("localhost"), port(27017) { }
  Database(const std::vector<std::string>& toks);
  
  const std::string& Name() const { return name; }
  const std::string& Address() const { return address; }
  int Port() const { return port; }
  const std::string& Login() const { return login; }
  const std::string& Password() const { return password; }
};

class Right : public Setting
{
  fs::Path path;
  // includes wildcards and possibley regex so can't be fs::Path path;
  acl::ACL acl;
  bool specialVar;
  
public:
  Right(std::vector<std::string> toks);
  const acl::ACL& ACL() const { return acl; }
  const fs::Path& Path() const { return path; }
  bool SpecialVar() const { return specialVar; }
};

class ACLInt : public Setting
{
  int arg;
  acl::ACL acl;
  
public:
  ACLInt(std::vector<std::string> toks);
  const acl::ACL& ACL() const { return acl; }
  int Arg() const { return arg; }
};

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
  SimXfers() : maxDownloads(-1), maxUploads(-1) { }
  SimXfers(std::vector<std::string> toks);
  int MaxDownloads() const { return maxDownloads; }
  int MaxUploads() const { return maxUploads; }
};

class PasvAddr : public Setting
{
  std::string addr;
public:
  PasvAddr(const std::vector<std::string>& toks);
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
 
class Ports : public Setting
{
  std::vector<PortRange> ranges;
public:
  Ports() {}
  Ports(const std::vector<std::string>& toks);
  const std::vector<PortRange>& Ranges() const { return ranges; }
};

class AllowFxp : public Setting
{
  bool downloads;
  bool uploads;
  bool logging;
  acl::ACL acl;
  
public:
  AllowFxp() :
    downloads(true), uploads(true), 
    logging(false), acl(acl::ACL::FromString("*"))
  { }
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
  Alias(const std::vector<std::string>& toks);
  const std::string& Name() const { return name; }
  const fs::Path& Path() const { return path; }
};

class PathFilter : public Setting
{
  fs::Path messagePath;
  boost::regex regex;
  acl::ACL acl;
  
public:
  PathFilter(std::vector<std::string> toks);
  const fs::Path& MessagePath() const { return messagePath; }
  const boost::regex& Regex() const { return regex; }
  const acl::ACL& ACL() const { return acl; }
  
};

class MaxUsers : public Setting
{
  int users;
  int exemptUsers;
public:
  MaxUsers() : users(50), exemptUsers(5) {}
  MaxUsers(const std::vector<std::string>& toks);
  int Users() const { return users; }
  int ExemptUsers() const { return exemptUsers; }
};

class ShowTotals : public Setting
{
  int maxLines;
  std::vector<std::string> paths;
public:
  ShowTotals(std::vector<std::string> toks);
  int MaxLines() const { return maxLines; }
};

class Lslong : public Setting
{
  fs::Path bin;
  std::string options;
  int maxRecursion;
public:
  Lslong() : options("l"), maxRecursion(0) { }
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
  HiddenFiles(std::vector<std::string> toks);
  const fs::Path& Path() const { return path; }
  const std::vector<std::string>& Masks() const { return masks; }
};

class Requests : public Setting
{
  fs::Path path;
  int max;
public:
  Requests() : max(10) { }
  Requests(const std::vector<std::string>& toks);
  const fs::Path& Path() const { return path; }                              
  int Max() const { return max; }
};

class Lastonline : public Setting
{
public:
  enum Type { ALL, TIMEOUT, ALL_WITH_ACTIVITY };

private:
  Type type;
  int max;

public:
  Lastonline() : type(ALL), max(10) {}
  Lastonline(const std::vector<std::string>& toks);
  Type GetType() const { return type; }
  int Max() const { return max; }
};

class Creditcheck : public Setting
{
  fs::Path path;
  int ratio;
  acl::ACL acl;
  
public:
  Creditcheck(std::vector<std::string> toks);
  const fs::Path& Path() const { return path; }
  int Ratio() const { return ratio; }
  const acl::ACL& ACL() const { return acl; }
};

class Creditloss : public Setting
{
  int ratio;
  bool allowLeechers;
  fs::Path path;
  acl::ACL acl;
  
public:
  Creditloss(std::vector<std::string> toks);
  const acl::ACL& ACL() const { return acl; }
  int Ratio() const { return ratio; }
  bool AllowLeechers() const { return allowLeechers; }
  const fs::Path& Path() const { return path; }
};

class NukedirStyle : public Setting
{
public:
  enum Type { DELETE_ALL, DELETE_FILES, KEEP };

private:
  std::string format;
  Type type;
  int minBytes;

public:
  NukedirStyle() : type(KEEP), minBytes(1024 * 1024) { }
  NukedirStyle(const std::vector<std::string>& toks);
  const std::string& Format() const { return format; }
  int MinBytes() const { return minBytes; }
  Type GetType() const { return type; }
};

class Msgpath : public Setting
{
  std::string path;
  fs::Path file;
  acl::ACL acl;
  
public:
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
  Privpath(std::vector<std::string> toks);
  const fs::Path& Path() const { return path; }
  const acl::ACL& ACL() const { return acl; }
};

class SiteCmd : public Setting
{
public:
  enum class Type { EXEC, TEXT, ALIAS };
  
private:
  std::string command;
  Type type;
  std::string description;
  std::string target;
  std::string arguments;

public:
  SiteCmd(const std::vector<std::string>& toks);
  const std::string& Command() const { return command; }
  Type GetType() const { return type; }
  const std::string& Description() const { return description; }
  const std::string& Arguments() const { return arguments; }
  const std::string& Target() const { return target; }
};

class Cscript : public Setting
{
public:
  enum class Type { PRE, POST };

private:
  std::string command;
  Type type;
  fs::Path path;
  
public:
  Cscript(const std::vector<std::string>& toks);
  const std::string& Command() const { return command; }                       
  const fs::Path& Path() const { return path; }
  Type GetType() const { return type; }
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

class CheckScript : public Setting
{
  fs::Path path;
  fs::Path mask;
  bool disabled;

public:
  CheckScript(const std::vector<std::string>& toks);

  const fs::Path Path() const { return path; }
  const fs::Path Mask() const { return mask; }
  bool Disabled() const { return disabled; }
};

// end namespace
}
}
#endif
