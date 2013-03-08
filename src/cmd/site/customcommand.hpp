#ifndef __CMD_SITE_CUSTOMCOMMAND_HPP
#define __CMD_SITE_CUSTOMCOMMAND_HPP

#include "cmd/command.hpp"
#include "cfg/setting.hpp"
#include "plugin/hooks.hpp"

namespace cmd { namespace site
{

class CustomCommand : public Command
{
protected:
  cfg::SiteCmd custSiteCmd;
  
public:
  CustomCommand(cfg::SiteCmd& custSiteCmd, ftp::Client& client, 
                const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args),
    custSiteCmd(custSiteCmd) { }

  virtual void Execute() = 0;
};

class CustomEXECCommand : public CustomCommand
{
public:
  CustomEXECCommand(cfg::SiteCmd& custSiteCmd, ftp::Client& client, 
                    const std::string& argStr, const Args& args) :
    CustomCommand(custSiteCmd, client, argStr, args) { }
    
  void Execute();
};

class CustomTEXTCommand : public CustomCommand
{
public:
  CustomTEXTCommand(cfg::SiteCmd& custSiteCmd, ftp::Client& client, 
                    const std::string& argStr, const Args& args) :
    CustomCommand(custSiteCmd, client, argStr, args) { }

  void Execute();
};


class CustomALIASCommand : public CustomCommand
{
public:
  CustomALIASCommand(cfg::SiteCmd& custSiteCmd, ftp::Client& client, 
                    const std::string& argStr, const Args& args) :
    CustomCommand(custSiteCmd, client, argStr, args) { }

  void Execute();
};

class PluginCommand : public cmd::Command
{
  plugin::Plugin& plugin;
  plugin::CommandHookFunction function;
  
public:
  PluginCommand(ftp::Client& client, const std::string& argStr, const cmd::Args& args,
                plugin::Plugin& plugin, const plugin::CommandHookFunction& function) :
    cmd::Command(client, client.Control(), client.Data(), argStr, args),
    plugin(plugin),
    function(function)
  {
  }
  
  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
