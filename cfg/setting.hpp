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
class Setting // a base class might be useful in the future.. maybe
{
};

// generics
class Right : public Setting
{
  std::string path;
  // includes wildcards and possibley regex so can't be fs::Path path;
  acl::ACL acl;
public:
  Right() {};
  virtual void Save(std::vector<std::string>& toks);
};

class ACLInt : public Setting
{
  int arg;
  acl::ACL acl;
public:
  ACLInt(std::vector<std::string>& toks);
};


// glftpd

class AsciiDownloads : public Setting
{
  int size;
  std::vector<std::string> masks;
public:
  AsciiDownloads(std::vector<std::string>& toks);
};

class UseDirSize : public Setting
{
  char unit;
  std::vector<fs::Path> paths;
public:
  UseDirSize(std::vector<std::string>& toks);
};

class Timezone : public Setting
{
  unsigned int hours;
public:
  Timezone() {};
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

class PasvAddr : public Setting
{
  std::string addr;
  bool nat;
public:
  PasvAddr(std::vector<std::string>& toks);
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

class Alias : public Setting
{
  std::string alias;
  fs::Path path;
public:
  Alias() {};
  virtual void Save(std::vector<std::string>& toks);
};

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

class ShowTotals : public Setting
{
  unsigned int maxLines;
  std::vector<std::string> paths;
public:
  ShowTotals() {};
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

class Requests : public Setting
{
  fs::Path path;
  int max;
public:
  Requests() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Lastonline : public Setting
{
  enum Type { ALL, TIMEOUT, ALL_WITH_ACTIVITY };
  Type type;
  int max;
public:
  Lastonline() {};
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

class NukedirStyle : public Setting
{
  std::string format;
  enum Type { DELETE_ALL, DELETE_FILES, KEEP };
  Type type;
  int minBytes;
public:
  NukedirStyle() {};
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

class SiteCmd : public Setting
{
  std::string command;
  enum Type { EXEC, TEXT, IS };
  Type type;
  std::string script;
  std::vector<std::string> arguments;
public:
  SiteCmd() {};
  virtual void Save(std::vector<std::string>& toks);
};

class Cscript : public Setting
{
  std::string command;
  enum Type { PRE, POST };
  Type type;
  fs::Path script;
public:
  Cscript() {};
  virtual void Save(std::vector<std::string>& toks);
};

// end namespace
}
}
#endif
