#ifndef __CMD_SITE_FACTORY_HPP
#define __CMD_SITE_FACTORY_HPP

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <boost/optional.hpp>
#include "util/string.hpp"
#include "cmd/command.hpp"
#include "ftp/client.hpp"
#include "cfg/setting.hpp"

namespace cmd { namespace site
{

template <class BaseT>
class CreatorBase
{
public:  
  virtual ~CreatorBase() { }
  
  virtual BaseT* Create(ftp::Client& client, const std::string& argStr,
                        const Args& args) = 0;
};

template <class CommandT>
class Creator : public CreatorBase<cmd::Command>
{
public:
  cmd::Command* Create(ftp::Client& client, const std::string& argStr,
                  const Args& args)
  {
    return new CommandT(client, argStr, args);
  }
};

template <class CommandT>
class CustomCreator : public CreatorBase<cmd::Command>
{
  cfg::setting::SiteCmd custSiteCmd;
  
public:
  CustomCreator(const cfg::setting::SiteCmd& custSiteCmd) : custSiteCmd(custSiteCmd) { }
  cmd::Command* Create(ftp::Client& client, const std::string& argStr, const Args&)
  {
    // prepend custom command arguments to user passed arguments
    std::string cArgStr(custSiteCmd.Arguments());
    if (!argStr.empty())
    {
      cArgStr += ' ';
      cArgStr += argStr;
    }
    
    // rebuild args
    std::vector<std::string> cArgs;
    if (!cArgStr.empty())
      util::Split(cArgs, cArgStr, " ", true);
    cArgs.insert(cArgs.begin(), custSiteCmd.Command());
    
    return new CommandT(custSiteCmd, client, cArgStr, cArgs);
  }
};

typedef std::shared_ptr<CreatorBase<cmd::Command>> CreatorBasePtr;

class CommandDef
{
  int minimumArgs;
  int maximumArgs;
  std::string aclKeyword;
  CreatorBasePtr creator;
  std::string syntax;
  std::string description;

public:
  CommandDef() :
    minimumArgs(-1), maximumArgs(-1), creator(nullptr) { }
  
  CommandDef(int minimumArgs, int maximumArgs,
             const std::string& aclKeyword,
             const CreatorBasePtr& creator,
             const std::string& syntax,
             const std::string& description) :
    minimumArgs(minimumArgs),
    maximumArgs(maximumArgs),
    aclKeyword(aclKeyword),
    creator(creator),
    syntax(syntax),
    description(description)
  { }
  
  CommandDef(const std::string& aclKeyword,
             const CreatorBasePtr& creator) :
    minimumArgs(0), maximumArgs(-1), aclKeyword(aclKeyword), 
    creator(creator) { }
  
  bool CheckArgs(const std::vector<std::string>& args) const
  {
    int argsSize = static_cast<int>(args.size()) - 1;
    return (argsSize >= minimumArgs &&
            (maximumArgs == -1 || argsSize <= maximumArgs));
  }
  
  CommandPtr Create(ftp::Client& client, const std::string& argStr, const Args& args) const
  {
    if (!creator) return nullptr;
    return CommandPtr(creator->Create(client, argStr, args));
  }
  
  const std::string& Syntax() const { return syntax; }
  const std::string& Description() const { return description; }
  const std::string& ACLKeyword() const { return aclKeyword; }
};

typedef boost::optional<CommandDef> CommandDefOpt;

class Factory
{
public:
  typedef std::unordered_map<std::string, CommandDef> CommandDefsMap;

private:                                   
  CommandDefsMap defs;
   
  Factory();
  
  static std::unique_ptr<Factory> factory;
 
public:
  static CommandDefOpt LookupCustom(const std::string& command);
  static CommandDefOpt Lookup(const std::string& command, bool noCustom = false);
  
  static void Initialise() { factory.reset(new Factory()); }
  static const CommandDefsMap& Commands() { return factory->defs; }
  static std::unordered_set<std::string> ACLKeywords();
};

} /* site namespace */
} /* cmd namespace */

#endif
