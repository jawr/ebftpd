#ifndef __CFG_SETTING_HPP
#define __CFG_SETTING_HPP
#include <string>
#include <vector>
#include "fs/path.hpp"
namespace cfg
{
class Setting {};

namespace setting
{

// Path settings
class Path : virtual public Setting
{
  fs::Path path;
public:
  Path(const std::string& path) : path(path) {};
  ~Path() {};
};

class PathWithArgument : virtual public Setting
{
  fs::Path path;
  std::string argument;
public:
  PathWithArgument(const std::string& path, const std::string& argument) :
    path(path), argument(argument) {};
  ~PathWithArgument() {};
};

// ACL settings
class ACLWithInt : virtual public Setting
{
  std::vector<std::string> acl;
  int argument;
public:
  ACLWithInt(int argument, std::vector<std::string>& acl) : acl(acl), 
    argument(argument) {};
  ~ACLWithInt() {};
};

class ACLWithArgument : virtual public Setting
{
  std::string argument;
  std::vector<std::string> acl;
public:
  ACLWithArgument(const std::string& argument, std::vector<std::string>& acl) :
    argument(argument), acl(acl) {};
  ~ACLWithArgument() {};
};

class ACLWithPath : virtual public Setting
{
  fs::Path path;
  std::vector<std::string> acl;
public:
  ACLWithPath(const std::string& path, 
    std::vector<std::string>& acl) : path(path), acl(acl) {};
  ~ACLWithPath() {};
};

class ACL : virtual public Setting
{
  std::vector<std::string> acl;
public:
  ACL(std::vector<std::string>& acl) : acl(acl) {};
  ~ACL() {};
};

// Misc
class StatSection : virtual public Setting
{
  std::string keyword;
  fs::Path path;
  bool seperateCredits;
public:
  StatSection(const std::string& keyword, const std::string& path,
     bool seperateCredits) : keyword(keyword), path(path), 
      seperateCredits(seperateCredits) {};
  ~StatSection() {};
};

class PathFilter : virtual public Setting
{
  std::string group;
  fs::Path messageFile;
  std::vector<std::string> filters;
public:
  PathFilter(const std::string& group, const std::string& messageFile,
    const std::vector<std::string>& filters) : 
      group(group), messageFile(messageFile), filters(filters) {};
  ~PathFilter() {};
};

enum When { PRE, POST };

class Script : virtual public Setting
{
  std::string script;
  When when;
  fs::Path path;
public:
  Script(const std::string& script, When when, const std::string& path) :
    script(script), when(when), path(path) {};
  ~Script() {};
};

class SpeedLimit : virtual public Setting
{
  fs::Path path;
  int upload;
  int download;
  std::string acl;
public:
  SpeedLimit(const std::string& path, int upload, int download, 
    const std::string& acl) : path(path), upload(upload), download(download),
    acl(acl) {};
  ~SpeedLimit() {};
};

class Requests : virtual public Setting
{
  fs::Path path;
  int lines;
public:
  Requests(const std::string& path, int lines) : path(path), lines(lines) {};
  ~Requests() {};
};

class IntWithArguments : virtual public Setting
{
  int first;
  std::vector<std::string> arguments;
public:
  IntWithArguments(int first, std::vector<std::string>& arguments) :
    first(first), arguments(arguments) {};
  ~IntWithArguments() {};
};

class IntWithBool : virtual public Setting
{
  int first;
  bool enabled;
public:
  IntWithBool(int first, bool enabled=false) : first(first), enabled(enabled) {};
  ~IntWithBool() {};
};

class NukedirStyle : virtual public Setting
{
  std::string format;
  int method;
  int bytes;
public:
  NukedirStyle(const std::string& format, int method, int bytes) :
    format(format), method(method), bytes(bytes) {};
  ~NukedirStyle() {};
};

class SecureIP : virtual public Setting
{
  int fields;
  bool allowHostnames;
  bool needIdent;
  std::string acl;
public:
  SecureIP(int fields, bool allowHostnames, bool needIdent,
    const std::string& acl) :
      fields(fields), allowHostnames(allowHostnames), needIdent(needIdent),
      acl(acl) {};
  ~SecureIP() {};
};

class PasvAddr : virtual public Setting
{
  std::string addr;
  bool primary;
public:
  PasvAddr(const std::string addr, bool primary=false) : addr(addr), 
    primary(primary) {};
  ~PasvAddr() {};
};

class AllowFxp : virtual public Setting
{
  bool downloads;
  bool uploads;
  bool logging;
  std::string acl;
public:
  AllowFxp(bool downloads, bool uploads, bool logging, const std::string& acl) :
    downloads(downloads), uploads(uploads), logging(logging), acl(acl) {};
  ~AllowFxp() {};
};

class CreditLoss : virtual public Setting
{
  int multiplier;
  bool leechers;
  fs::Path path;
  std::vector<std::string> acl;
public:
  CreditLoss(int multiplier, bool leechers, const std::string& path, 
    std::vector<std::string>& acl) : multiplier(multiplier), leechers(leechers),
      path(path), acl(acl) {};
  ~CreditLoss() {};
};

class Creditcheck : virtual public Setting
{
  fs::Path path;
  int ratio;
  std::vector<std::string> acl;
public:
  Creditcheck(const std::string& path, int ratio, std::vector<std::string>& acl) :
    path(path), ratio(ratio), acl(acl) {};
  ~Creditcheck() {};
};

class MsgPath : virtual public Setting
{
  fs::Path path;
  std::string filename;
  std::vector<std::string> acl;
public:
  MsgPath(const std::string& path, const std::string& filename, 
    std::vector<std::string>& acl) : path(path), filename(filename), acl(acl) {};
  ~MsgPath() {};
};

class Cscript : virtual public Setting
{
  std::string name;
  std::string command;
  fs::Path path;
public:
  Cscript(const std::string& name, const std::string& command, 
    const std::string& path) : name(name), command(command), path(path) {};
  ~Cscript() {};
};

  
enum SiteCmdMethod { EXEC, TEXT, IS };

class SiteCmd : virtual public Setting
{
  std::string command;
  SiteCmdMethod method;
  fs::Path file;
  std::vector<std::string> arguments;
  std::vector<std::string> acl;
public:
  // initalize with acl
  SiteCmd(const std::vector<std::string>& acl) : acl(acl), command(), method(EXEC),
    file("/error/unconfigured"), arguments() {};
  // initalize with site_cmd 
  SiteCmd(const std::string& command, SiteCmdMethod method, 
    const std::string& file, const std::vector<std::string>& arguments) :
    acl(), command(command), method(method), file(file), arguments(arguments) {};
  // set alternative
  void SetSiteCmd(const std::string& command, SiteCmdMethod method, 
    const std::string& file, const std::vector<std::string>& arguments)
  {
    this->command = command;
    this->method  = method;
    this->file = fs::Path(file);
    this->arguments = arguments;
  };
  void SetACL(const std::vector<std::string>& acl)
  {
    this->acl = acl;
  };
  ~SiteCmd() {};
};
  

  
} // end setting
} // end cfg
#endif
