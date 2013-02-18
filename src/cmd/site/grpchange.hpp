#ifndef __CMD_SITE_GRPCHANGE_HPP
#define __CMD_SITE_GRPCHANGE_HPP

#include <functional>
#include <string>
#include "cmd/command.hpp"

namespace acl
{
class Group;
}

namespace cmd { namespace site
{

class GRPCHANGECommand;

class GRPCHANGECommand : public Command
{
  typedef std::function<void(acl::Group&)> SetFunction;
  typedef std::function<SetFunction(GRPCHANGECommand*)> CheckFunction;

  struct SettingDef
  {
    std::string name;
    int maximumArgs;
    std::string aclKeyword;
    CheckFunction check;
    std::string description;
  };
  
  std::string display;

  static const std::vector<SettingDef> settings;
  
  SetFunction CheckSlots();
  SetFunction CheckLeechSlots();
  SetFunction CheckAllotSlots();
  SetFunction CheckMaxAllotSize();
  SetFunction CheckMaxLogins();
  SetFunction CheckDescription();
  SetFunction CheckComment();
  
  SetFunction Check();

public:
  GRPCHANGECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
  
  static std::string Syntax();
};

} /* site namespace */
} /* cmd namespace */

#endif

