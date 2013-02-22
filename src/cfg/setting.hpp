#ifndef __CFG_SETTING_HPP
#define __CFG_SETTING_HPP

#include <string>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
#include <sys/types.h>
#include "acl/acl.hpp"
#include "acl/passwdstrength.hpp"
#include "acl/ipstrength.hpp"
#include "main.hpp"

namespace cfg { namespace setting
{

class Database
{
  std::string name;
  std::string address;
  std::string host;
  int port;
  std::string login;
  std::string password;
  
public:
  Database();
  Database(const std::vector<std::string>& toks);
  
  const std::string& Name() const { return name; }
  const std::string& Address() const { return address; }
  int Port() const { return port; }
  const std::string& Host() const { return host; }
  const std::string& Login() const { return login; }
  const std::string& Password() const { return password; }
  bool NeedAuth() const;
};

class Right
{
  std::string path;
  // includes wildcards and possibley regex so can't be std::string path;
  acl::ACL acl;
  bool specialVar;
  
public:
  Right(std::vector<std::string> toks);
  const acl::ACL& ACL() const { return acl; }
  const std::string& Path() const { return path; }
  bool SpecialVar() const { return specialVar; }
};

class ACLInt
{
  int arg;
  acl::ACL acl;
  
public:
  ACLInt(std::vector<std::string> toks);
  const acl::ACL& ACL() const { return acl; }
  int Arg() const { return arg; }
};

class AsciiDownloads
{
  off_t size;
  std::vector<std::string> masks;
  
public:
  AsciiDownloads() : size(-1) { }
  AsciiDownloads(const std::vector<std::string>& toks);
  bool Allowed(off_t size, const std::string& path) const;
};

class AsciiUploads
{
  std::vector<std::string> masks;
  
public:
  AsciiUploads() = default;
  AsciiUploads(const std::vector<std::string>& toks);
  bool Allowed(const std::string& path) const;
};

class SecureIp
{
  acl::IPStrength strength;
  acl::ACL acl;

public:
  SecureIp(std::vector<std::string> toks);
  const acl::IPStrength& Strength() const { return strength; }
  const acl::ACL& ACL() const { return acl; }
};

class SecurePass
{
  acl::PasswdStrength strength;
  acl::ACL acl;
public:
  SecurePass(std::vector<std::string> toks);
  const acl::ACL& ACL() const { return acl; }
  const acl::PasswdStrength& Strength() const { return strength; }
};

class BouncerIp
{
  std::vector<std::string> addrs;
public:
  BouncerIp(const std::vector<std::string>& toks);
  const std::vector<std::string>& Addrs() const { return addrs; }
};

class SpeedLimit
{
  std::string path;
  long dlLimit;
  long ulLimit;
  acl::ACL acl;
public:
  SpeedLimit(std::vector<std::string> toks);
  const std::string& Path() const { return path; }
  long DlLimit() const { return dlLimit; }
  long UlLimit() const { return ulLimit; }
  const acl::ACL& ACL() const { return acl; }
};

class SimXfers
{
  int maxDownloads;
  int maxUploads;
  
public:
  SimXfers() : maxDownloads(-1), maxUploads(-1) { }
  SimXfers(std::vector<std::string> toks);
  int MaxDownloads() const { return maxDownloads; }
  int MaxUploads() const { return maxUploads; }
};

class PasvAddr
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
  PortRange(int from, int to) : from(from), to(to) { }
  int From() const { return from; }
  int To() const { return to; }
};
 
class Ports
{
  std::vector<PortRange> ranges;
public:
  Ports() = default;
  Ports(const std::vector<std::string>& toks);
  const std::vector<PortRange>& Ranges() const { return ranges; }
};

class AllowFxp
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

class Alias
{
  std::string name;
  std::string path;
public:
  Alias(const std::vector<std::string>& toks);
  const std::string& Name() const { return name; }
  const std::string& Path() const { return path; }
};

class PathFilter
{
  std::string messagePath;
  boost::regex regex;
  acl::ACL acl;
  
public:
  PathFilter(std::vector<std::string> toks);
  const std::string& MessagePath() const { return messagePath; }
  const boost::regex& Regex() const { return regex; }
  const acl::ACL& ACL() const { return acl; }
  
};

class MaxUsers
{
  int users;
  int exemptUsers;
public:
  MaxUsers() : users(50), exemptUsers(5) { }
  MaxUsers(const std::vector<std::string>& toks);
  int Users() const { return users; }
  int ExemptUsers() const { return exemptUsers; }
};

class ShowTotals
{
  int maxLines;
  std::vector<std::string> paths;
public:
  ShowTotals(std::vector<std::string> toks);
  int MaxLines() const { return maxLines; }
};

class Lslong
{
  std::string bin;
  std::string options;
  int maxRecursion;
public:
  Lslong() : options("l"), maxRecursion(0) { }
  Lslong(std::vector<std::string> toks);
  const std::string& Bin() const { return bin; }
  const std::string& Options() const { return options; }
  int MaxRecursion() const { return maxRecursion; }
};

class HiddenFiles
{
  std::string path;
  std::vector<std::string> masks;
public:
  HiddenFiles(std::vector<std::string> toks);
  const std::string& Path() const { return path; }
  const std::vector<std::string>& Masks() const { return masks; }
};

class Requests
{
  std::string path;
  int max;
public:
  Requests() : max(10) { }
  Requests(const std::vector<std::string>& toks);
  const std::string& Path() const { return path; }                              
  int Max() const { return max; }
};

class Lastonline
{
public:
  enum Type { ALL, TIMEOUT, ALL_WITH_ACTIVITY };

private:
  Type type;
  int max;

public:
  Lastonline() : type(ALL), max(10) { }
  Lastonline(const std::vector<std::string>& toks);
  Type GetType() const { return type; }
  int Max() const { return max; }
};

class Creditcheck
{
  std::string path;
  int ratio;
  acl::ACL acl;
  
public:
  Creditcheck(std::vector<std::string> toks);
  const std::string& Path() const { return path; }
  int Ratio() const { return ratio; }
  const acl::ACL& ACL() const { return acl; }
};

class Creditloss
{
  int ratio;
  bool allowLeechers;
  std::string path;
  acl::ACL acl;
  
public:
  Creditloss(std::vector<std::string> toks);
  const acl::ACL& ACL() const { return acl; }
  int Ratio() const { return ratio; }
  bool AllowLeechers() const { return allowLeechers; }
  const std::string& Path() const { return path; }
};

class NukedirStyle
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

class Msgpath
{
  std::string path;
  std::string file;
  acl::ACL acl;
  
public:
  Msgpath(const std::vector<std::string>& toks);
  const std::string& Path() const { return path; }
  const acl::ACL& ACL() const { return acl; }
  const std::string& File() const { return file; }
};

class Privpath
{
  std::string path; // no wildcards to avoid slowing down listing
  acl::ACL acl;

public:
  Privpath(std::vector<std::string> toks);
  const std::string& Path() const { return path; }
  const acl::ACL& ACL() const { return acl; }
};

class SiteCmd
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

class Cscript
{
public:
  enum class Type { PRE, POST };

private:
  std::string command;
  Type type;
  std::string path;
  
public:
  Cscript(const std::vector<std::string>& toks);
  const std::string& Command() const { return command; }                       
  const std::string& Path() const { return path; }
  Type GetType() const { return type; }
};

class IdleTimeout
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

class CheckScript
{
  std::string path;
  std::string mask;
  bool disabled;

public:
  CheckScript(const std::vector<std::string>& toks);

  const std::string Path() const { return path; }
  const std::string Mask() const { return mask; }
  bool Disabled() const { return disabled; }
};

class Log
{
  std::string filename;
  bool console;
  bool file;
  long database;
  
public:
  Log(const std::string& filename, bool console, bool file, long database) : 
    filename(filename), console(console), file(file), database(database)
  { }
  
  Log(const std::string& filename, const std::vector<std::string>& toks);
  
  const std::string& Filename() const { return filename; }
  bool Console() const { return console; }
  bool File() const { return file; }
  long Database() const { return database; };
};

// end namespace
}
}
#endif
