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

#ifndef __CFG_SETTING_HPP
#define __CFG_SETTING_HPP

#include <string>
#include <vector>
#include <memory>
#include <boost/regex_fwd.hpp>
#include <sys/types.h>
#include "acl/acl.hpp"
#include "acl/passwdstrength.hpp"
#include "acl/ipstrength.hpp"
#include "main.hpp"

namespace boost { namespace posix_time
{
class seconds;
}
}

namespace cfg
{

class Config;

class Database
{
  std::string name;
  std::vector<std::pair<std::string, int>> hosts;
  std::string login;
  std::string password;
  std::string replicaSet;
  
public:
  Database() = default;
  Database(const char* name, const char* address, int port, const char* login, const char* password);
  
  const std::string& Name() const { return name; }
  std::string URL() const;
  const std::string& Login() const { return login; }
  const std::string& Password() const { return password; }
  bool NeedAuth() const;
  
  bool operator==(const Database& rhs) const;
  bool operator!=(const Database& rhs) const { return !operator==(rhs); }
  
  friend class Config;
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
  long long kBytes;
  std::vector<std::string> masks;
  
public:
  AsciiDownloads() : kBytes(-1) { }
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
  long long downloads;
  long long uploads;
  acl::ACL acl;
  
public:
  SpeedLimit(std::vector<std::string> toks);
  const std::string& Path() const { return path; }
  long long Uploads() const { return downloads; }
  long long Downloads() const { return uploads; }
  const acl::ACL& ACL() const { return acl; }
};

class SimXfers
{
  int maxDownloads;
  int maxUploads;
  
public:
  SimXfers(int maxDownloads, int maxUploads);
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
  AllowFxp(bool downloads, bool uploads, bool logging, const char* acl);
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
  std::unique_ptr<boost::regex> regex;
  acl::ACL acl;
  
public:
  PathFilter& operator=(const PathFilter& rhs);
  PathFilter& operator=(PathFilter&& rhs);
  PathFilter(const PathFilter& other);
  PathFilter(PathFilter&& other);
  ~PathFilter();
  
  PathFilter(const char* regex, const char* acl);
  PathFilter(std::vector<std::string> toks);
  const boost::regex& Regex() const;
  const acl::ACL& ACL() const { return acl; }
  
};

class MaxUsers
{
  int users;
  int exemptUsers;
  
public:
  MaxUsers(int users, int exemptUsers);
  MaxUsers(const std::vector<std::string>& toks);
  int Users() const { return users; }
  int ExemptUsers() const { return exemptUsers; }
  int Total() const { return users + exemptUsers; }

  bool operator==(const MaxUsers& rhs) const
  {
    return users == rhs.users && exemptUsers == rhs.exemptUsers;
  }
  
  bool operator!=(const MaxUsers& rhs) const { return !operator==(rhs); }
};

class Lslong
{
  std::string options;
  int maxRecursion;
  
public:
  Lslong(const char* options, int maxRecursion);
  Lslong(std::vector<std::string> toks);
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
  std::string path;
  int ratio;
  acl::ACL acl;
  
public:
  Creditloss(std::vector<std::string> toks);
  const acl::ACL& ACL() const { return acl; }
  int Ratio() const { return ratio; }
  const std::string& Path() const { return path; }
};

class NukeStyle
{
public:
  enum Action { DeleteAll, KeepDir, Keep };

private:
  std::string format;
  Action action;
  long long emptyKBytes;
  long long emptyPenalty;

public:
  NukeStyle(const std::string& format, Action action, 
            long long emptyKBytes, long long emptyPenalty);
  NukeStyle(const std::vector<std::string>& toks);
  const std::string& Format() const { return format; }
  long long EmptyKBytes() const { return emptyKBytes; }
  long long EmptyPenalty() const { return emptyPenalty; }
  Action GetAction() const { return action; }
};

class Msgpath
{
  std::string path;
  std::string filepath;
  acl::ACL acl;
  
public:
  Msgpath(const std::vector<std::string>& toks);
  const std::string& Path() const { return path; }
  const acl::ACL& ACL() const { return acl; }
  const std::string& Filepath() const { return filepath; }
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
  enum class Type { Exec, Text, Alias };
  
private:
  std::string command;
  std::string syntax;
  Type type;
  std::string description;
  std::string target;
  std::string arguments;

public:
  SiteCmd(const std::vector<std::string>& toks);
  const std::string& Command() const { return command; }
  const std::string& Syntax() const { return syntax; }
  Type GetType() const { return type; }
  const std::string& Description() const { return description; }
  const std::string& Arguments() const { return arguments; }
  const std::string& Target() const { return target; }
};

class Cscript
{
public:
  enum class Type { Pre, Post };

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

struct IdleTimeoutImpl;

class IdleTimeout
{
  std::unique_ptr<IdleTimeoutImpl> pimpl;
  
public:
  IdleTimeout& operator=(const IdleTimeout& rhs);
  IdleTimeout& operator=(IdleTimeout&& rhs);
  IdleTimeout(const IdleTimeout& other);
  IdleTimeout(IdleTimeout&& other);

  IdleTimeout(long maximum, long minimum, long timeout);
  IdleTimeout(const std::vector<std::string>& toks);
  ~IdleTimeout();
    
  boost::posix_time::seconds Maximum() const;
  boost::posix_time::seconds Minimum() const;
  boost::posix_time::seconds Timeout() const;
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
  std::string name;
  bool console;
  bool file;
  long database;
  
public:
  Log(const std::string& name, bool console, bool file, long database) : 
    name(name), console(console), file(file), database(database)
  { }
  
  Log(const std::string& name, const std::vector<std::string>& toks);
  
  const std::string& Name() const { return name; }
  bool Console() const { return console; }
  bool File() const { return file; }
  bool Database() const { return database > 0; };
  long CollectionSize() const { return database; }
};

class TransferLog : public Log
{
  bool uploads;
  bool downloads;
  
public:
  TransferLog(const std::string& name, bool console, bool file, 
              long database, bool uploads, bool downloads) : 
    Log(name, console, file, database),
    uploads(uploads), downloads(downloads)
  { }
  
  TransferLog(const std::string& name, const std::vector<std::string>& toks);
  
  bool Uploads() const { return uploads; }
  bool Downloads() const { return downloads; }
};

class NukeMax
{
  int multiplier;
  int percent;
  
public:
  NukeMax(int multiplier, int percent) :
    multiplier(multiplier),
    percent(percent)
  { }
  
  NukeMax(const std::vector<std::string>& toks);
  
  int Multiplier() const { return multiplier; }
  int Percent() const { return percent; }
  
  bool IsOkay(int value, bool isPercent) const;
};

}

#endif
