#ifndef __CMD_SITE_FACTORY_HPP
#define __CMD_SITE_FACTORY_HPP

#include <memory>
#include <unordered_map>
#include <boost/optional.hpp>
#include "cmd/command.hpp"
#include "ftp/client.hpp"

namespace cmd { namespace site
{

template <class BaseT>
class CreatorBase
{
public:  
  CreatorBase() { }
  virtual ~CreatorBase() { }
  
  virtual BaseT *Create(ftp::Client& client, const std::string& argStr,
                        const Args& args) = 0;
};

template <class CommandT>
class Creator : public CreatorBase<cmd::Command>
{
public:
  Creator() { }
  cmd::Command *Create(ftp::Client& client, const std::string& argStr,
                  const Args& args)
  {
    return new CommandT(client, argStr, args);
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
             CreatorBase<cmd::Command>* creator,
             const std::string& syntax,
             const std::string& description) :
    minimumArgs(minimumArgs),
    maximumArgs(maximumArgs),
    aclKeyword(aclKeyword),
    creator(creator),
    syntax(syntax),
    description(description)
  { }
  
  bool CheckArgs(const std::vector<std::string>& args) const
  {
    int argsSize = static_cast<int>(args.size()) - 1;
    return (argsSize >= minimumArgs &&
            (maximumArgs == -1 || argsSize <= maximumArgs));
  }
  
  cmd::Command* Create(ftp::Client& client, const std::string& argStr,
                       const Args& args) const
  {
    return creator->Create(client, argStr, args);
  }
  
  const std::string& Syntax() const { return syntax; }
  const std::string& Description() const { return description; }
  const std::string& ACLKeyword() const { return aclKeyword; }
};

typedef boost::optional<const CommandDef&> CommandDefOptRef;

class Factory
{
  typedef std::unordered_map<std::string, CommandDef> CommandDefsMap;
                                   
  CommandDefsMap defs;
   
  Factory();
  
  //void Register(const std::string& command, CreatorBase<cmd::Command>* creator);
  
  static Factory factory;
  
public:
  static CommandDefOptRef Lookup(const std::string& command);
};

} /* site namespace */
} /* cmd namespace */

#endif
