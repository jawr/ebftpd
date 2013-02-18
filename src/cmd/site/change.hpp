#ifndef __CMD_SITE_CHANGE_HPP
#define __CMD_SITE_CHANGE_HPP

#include <functional>
#include <string>
#include "cmd/command.hpp"

namespace cmd { namespace site
{

class CHANGECommand;

class CHANGECommand : public Command
{
  typedef std::function<void(acl::User&)> SetFunction;
  typedef std::function<SetFunction(CHANGECommand*)> CheckFunction;

  struct SettingDef
  {
    std::string name;
    int maximumArgs;
    std::string aclKeyword;
    CheckFunction check;
    std::string description;
  };

  bool gadmin;
  std::string display;

  static const std::vector<SettingDef> settings;

  SetFunction CheckRatio();
  SetFunction CheckSectionRatio();
  SetFunction CheckWeeklyAllotment();
  SetFunction CheckHomeDir();
  SetFunction HasFlags();
  SetFunction CheckIdleTime();
  SetFunction CheckExpires();
  SetFunction CheckNumLogins();
  SetFunction CheckTagline();
  SetFunction CheckComment();
  SetFunction CheckMaxUpSpeed();
  SetFunction CheckMaxDownSpeed();
  SetFunction CheckMaxSimUp();
  SetFunction CheckMaxSimDown();
  
  SetFunction Check();
  
public:
  CHANGECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args), gadmin(false) { }

  void Execute();
  
  static std::string Syntax();
};

} /* site namespace */
} /* cmd namespace */

#endif


