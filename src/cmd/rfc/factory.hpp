#ifndef __CMD_RFC_FACTORY_HPP
#define __CMD_RFC_FACTORY_HPP

#include <memory>
#include <unordered_map>
#include "cmd/command.hpp"
#include "ftp/client.hpp"

namespace cmd { namespace rfc
{

template <class BaseT>
class CreatorBase
{
public:  
  CreatorBase() { }
  virtual ~CreatorBase() { }
  
  virtual BaseT* Create(ftp::Client& client, const std::string& argStr, const Args& args) = 0;
};

template <class CommandT>
class Creator : public CreatorBase<cmd::Command>
{
public:
  Creator() { }
  CommandT* Create(ftp::Client& client, const std::string& argStr, const Args& args)
  {
    return new CommandT(client, argStr, args);
  }
};

typedef std::shared_ptr<CreatorBase<cmd::Command>> CreatorBasePtr;

class CommandDef
{
  int minimumArgs;
  int maximumArgs;
  ftp::ClientState requiredState;
  CreatorBasePtr creator;
  std::string syntax;
  
public:
  CommandDef(int minimumArgs, int maximumArgs, 
             ftp::ClientState requiredState,
             const CreatorBasePtr& creator,
             const std::string& syntax) :
    minimumArgs(minimumArgs),
    maximumArgs(maximumArgs),
    requiredState(requiredState),
    creator(creator),
    syntax(syntax)
  {
  }
  
  bool CheckArgs(const std::vector<std::string>& args) const
  {
    int argsSize = static_cast<int>(args.size()) - 1;
    return (argsSize >= minimumArgs &&
            (maximumArgs == -1 || argsSize <= maximumArgs));
  }

  ftp::ClientState RequiredState() const { return requiredState; }
  const std::string& Syntax() const { return syntax; }
  
  cmd::CommandPtr Create(ftp::Client& client, const std::string& argStr,
                         const Args& args) const
  {
    if (!creator) return nullptr;
    return cmd::CommandPtr(creator->Create(client, argStr, args));
  }
};

typedef boost::optional<const CommandDef&> CommandDefOptRef;

class Factory
{
  typedef std::unordered_map<std::string, CommandDef> CommandDefMap;
                                   
  CommandDefMap defs;
   
  Factory();
  
  static Factory factory;
  
public:
  static CommandDefOptRef Lookup(const std::string& command);
};

} /* rfc namespace */
} /* cmd namespace */

#endif
