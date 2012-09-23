#ifndef __CFG_SETTING_HPP
#define __CFG_SETTING_HPP
#include <string>
#include <vector>
#include "fs/path.hpp"
namespace cfg
{
class Setting {};

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
  const fs::Path& Path() const { return path; };
};

class ACL : virtual public Setting
{
  std::vector<std::string> acl;
public:
  ACL(std::vector<std::string>& acl) : acl(acl) {};
  ACL(const std::string& acl) { acl.push_back(acl); };
  ~ACL() {};
};

// Misc
class StatSectionOpt : virtual public Setting
{
  std::string keyword;
  fs::Path path;
  bool seperateCredits;
public:
  StatSectionOpt(const std::string& keyword, const std::string& path,
     bool seperateCredits) : keyword(keyword), path(path), 
      seperateCredits(seperateCredits) {};
  ~StatSectionOpt() {};
};

class PathFilterOpt : virtual public Setting
{
  std::string group;
  fs::Path messageFile;
  std::vector<std::string> filters;
public:
  PathFilterOpt(const std::string& group, const std::string& messageFile,
    const std::vector<std::string>& filters) : 
      group(group), messageFile(messageFile), filters(filters) {};
  ~PathFilterOpt() {};
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

class SpeedLimitOpt : virtual public Setting
{
  fs::Path path;
  int upload;
  int download;
  std::vector<std::string> acl;
public:
  SpeedLimitOpt(const std::string& path, int upload, int download, 
    const std::vector<std::string>& acl) : path(path), upload(upload), download(download),
    acl(acl) {};
  ~SpeedLimitOpt() {};
};

class RequestsOpt : virtual public Setting
{
  fs::Path path;
  int lines;
public:
  RequestsOpt(const std::string& path, int lines) : path(path), lines(lines) {};
  ~RequestsOpt() {};
};

class IntWithArguments : virtual public Setting
{
  int first;
  std::vector<std::string> arguments;
public:
  IntWithArguments(int first) : first(first), arguments() {};
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

class NukedirStyleOpt : virtual public Setting
{
  std::string format;
  int method;
  int bytes;
public:
  NukedirStyleOpt(const std::string& format, int method, int bytes) :
    format(format), method(method), bytes(bytes) {};
  ~NukedirStyleOpt() {};
};

class SecureIpOpt : virtual public Setting
{
  int fields;
  bool allowHostnames;
  bool needIdent;
  std::vector<std::string> acl;
public:
  SecureIpOpt(int fields, bool allowHostnames, bool needIdent,
    const std::vector<std::string>& acl) :
      fields(fields), allowHostnames(allowHostnames), needIdent(needIdent),
      acl(acl) {};
  ~SecureIpOpt() {};
};

class PasvAddrOpt : virtual public Setting
{
  std::string addr;
  bool primary;
public:
  PasvAddrOpt(const std::string addr, bool primary=false) : addr(addr), 
    primary(primary) {};
  ~PasvAddrOpt() {};
  void SetPrimary() { primary = true; };
};

class AllowFxpOpt : virtual public Setting
{
  bool downloads;
  bool uploads;
  bool logging;
  std::vector<std::string> acl;
public:
  AllowFxpOpt(bool downloads, bool uploads, bool logging, const std::vector<std::string>& acl) :
    downloads(downloads), uploads(uploads), logging(logging), acl(acl) {};
  ~AllowFxpOpt() {};
};

class CreditlossOpt : virtual public Setting
{
  int multiplier;
  bool leechers;
  fs::Path path;
  std::vector<std::string> acl;
public:
  CreditlossOpt(int multiplier, bool leechers, const std::string& path, 
    std::vector<std::string>& acl) : multiplier(multiplier), leechers(leechers),
      path(path), acl(acl) {};
  ~CreditlossOpt() {};
};

class CreditcheckOpt : virtual public Setting
{
  fs::Path path;
  int ratio;
  std::vector<std::string> acl;
public:
  CreditcheckOpt(const std::string& path, int ratio, std::vector<std::string>& acl) :
    path(path), ratio(ratio), acl(acl) {};
  ~CreditcheckOpt() {};
};

class MsgPathOpt : virtual public Setting
{
  fs::Path path;
  std::string filename;
  std::vector<std::string> acl;
public:
  MsgPathOpt(const std::string& path, const std::string& filename, 
    std::vector<std::string>& acl) : path(path), filename(filename), acl(acl) {};
  ~MsgPathOpt() {};
};

class CscriptOpt : virtual public Setting
{
  std::string name;
  std::string command;
  fs::Path path;
public:
  CscriptOpt(const std::string& name, const std::string& command, 
    const std::string& path) : name(name), command(command), path(path) {};
  ~CscriptOpt() {};
};

  
enum SiteCmdMethod { EXEC, TEXT, IS };

class SiteCmdOpt : virtual public Setting
{
  std::string command;
  SiteCmdMethod method;
  fs::Path file;
  std::vector<std::string> arguments;
  std::vector<std::string> acl;
public:
  // initalize with acl
  SiteCmdOpt(const std::vector<std::string>& acl) :
    command(), method(EXEC), file("/error/unconfigured"),
    arguments(), acl(acl) {};
  // initalize with site_cmd 
  SiteCmdOpt(const std::string& command, SiteCmdMethod method, 
    const std::string& file, const std::vector<std::string>& arguments) :
    command(command), method(method), file(file), arguments(arguments), acl()  {};
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
  ~SiteCmdOpt() {};
};
  

  
} // end cfg
#endif
