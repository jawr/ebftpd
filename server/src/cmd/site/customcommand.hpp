#ifndef __CMD_SITE_CUSTOMCOMMAND_HPP
#define __CMD_SITE_CUSTOMCOMMAND_HPP

#include "cmd/command.hpp"
#include "cfg/setting.hpp"

namespace cmd { namespace site
{

class CustomCommand : public Command
{
protected:
  cfg::setting::SiteCmd custSiteCmd;
  
public:
  CustomCommand(cfg::setting::SiteCmd& custSiteCmd, ftp::Client& client, 
                const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args),
    custSiteCmd(custSiteCmd) { }

  virtual void Execute() = 0;
};

class CustomEXECCommand : public CustomCommand
{
public:
  CustomEXECCommand(cfg::setting::SiteCmd& custSiteCmd, ftp::Client& client, 
                    const std::string& argStr, const Args& args) :
    CustomCommand(custSiteCmd, client, argStr, args) { }
    
  void Execute();
};

class CustomTEXTCommand : public CustomCommand
{
public:
  CustomTEXTCommand(cfg::setting::SiteCmd& custSiteCmd, ftp::Client& client, 
                    const std::string& argStr, const Args& args) :
    CustomCommand(custSiteCmd, client, argStr, args) { }

  void Execute();
};


class CustomALIASCommand : public CustomCommand
{
public:
  CustomALIASCommand(cfg::setting::SiteCmd& custSiteCmd, ftp::Client& client, 
                    const std::string& argStr, const Args& args) :
    CustomCommand(custSiteCmd, client, argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
