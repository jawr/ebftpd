#ifndef __CFG_SETTING_HPP
#define __CFG_SETTING_HPP
#include <string>
#include <vector>
#include "acl/common.hpp"
#include "util/endpoint.hpp"
namespace cfg
{
class Setting {};

class Config
{
  std::string config;
  std::map<std::string, Setting> settings;
public:
  Config(const std::string& file);
  ~Config() {};
};

namespace setting
{

class Bool : virtual public Setting
{
  bool argument;
public:
  Bool(bool argument) : argument(argument) {};
  ~Bool() {};
};

class Arguments : virtual public Setting
{
  std::vector<std::string> arguments;
public:
  Arguments(const std::vector<std::string>& arguments) :
    arguments(arguments) {};
  ~Arguments() {};
};

class Path : virtual public Setting
{
  std::string path;
public:
  Path(const std::string& path) : path(path) {};
  ~Arguments() {};
};

class ACLWithArgs : virtual public Setting
{
  std::vector<std::string> arguments;
  std::string path;
  acl::ACL acl;
public:
  ACLWithArgs(const std::vector<std::string>& arguments, const std::string& path,
    const std::string& acl) : arguments(arguments), path(path), acl(acl) {};
  ~ACLWithArgs() {};
};

class ACLWithPath : virtual public Setting
{
  std::string path;
  acl::ACL acl;
public:
  ACLWithPath(const std::string& path, 
    const std::string& acl) : path(path), acl(acl) {};
  ~ACLWithPath() {};
};

class ACL : virtual public Setting
{
  acl::ACL acl;
public:
  ACL(const std::string& acl) : acl(acl) {};
  ~ACL() {};
};

class Bool : virtual public Setting
{
  bool arg;
public:
  Bool(bool arg) : arg(arg) {};
  ~Bool() {};
};

class Range : virtual public Setting
{
  int start;
  int end;
public:
  Range(int start, int end) : start(start), end(end) {};
  ~Range() {};
};

class Endpoint : virtual public Setting
{
  ::util::endpoint addr;
public:
  Endpoint(const ::util::endpoint& addr) : addr(addr) {};
  ~Endpoint() {};
};

class Endpoints : virtual public Setting
{
  std::vector<::util::endpoint> addrs;
public:
  Endpoints(std::vector<::util::endpoint>& addrs) : addrs(addrs) {};
  ~Endpoints() {};
};

class StatSection : virtual public Setting
{
  std::string keyword;
  std::string path;
  bool seperateCredits;
public:
  StatSection(const std::string& keyword, const std::string&path path,
    bool seperateCredits) : keyword(keyword), path(path), 
      seperateCredits(seperateCredits) {};
  ~StatSection() {};
};

class PathFilter : virtual public Setting
{
  std::string group;
  std::string messageFile;
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
  std::string path;
public:
  Script(const std::string& script, When when, const std::string& path) :
    script(script), when(when), path(path) {};
  ~Script() {};
};


} // end setting
} // end cfg
#endif
