//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
  cfg::SiteCmd custSiteCmd;
  
public:
  CustomCreator(const cfg::SiteCmd& custSiteCmd) : custSiteCmd(custSiteCmd) { }
  cmd::Command* Create(ftp::Client& client, const std::string& argStr, const Args&)
  {
    // prepend custom command arguments to user passed arguments
    std::string cArgStr(custSiteCmd.Arguments());
    if (!argStr.empty())
    {
      if (!cArgStr.empty()) cArgStr += ' ';
      cArgStr += argStr;
    }

    // rebuild args
    std::vector<std::string> cArgs;
    if (!cArgStr.empty()) util::Split(cArgs, cArgStr, " ", true);
    cArgs.insert(cArgs.begin(), custSiteCmd.Command());
    
    return new CommandT(custSiteCmd, client, cArgStr, cArgs);
  }
};

class CommandDef
{
  int minimumArgs;
  int maximumArgs;
  std::string aclKeyword;
  std::shared_ptr<CreatorBase<cmd::Command>> creator;
  std::string syntax;
  std::string description;

public:
  CommandDef(int minimumArgs, int maximumArgs,
             const std::string& aclKeyword,
             const std::shared_ptr<CreatorBase<cmd::Command>>& creator,
             const std::string& syntax,
             const std::string& description);
  CommandDef(const std::string& aclKeyword, 
             const std::string& syntax,
             const std::string& description,
             const std::shared_ptr<CreatorBase<cmd::Command>>& creator);
  bool CheckArgs(const std::vector<std::string>& args) const;
  CommandPtr Create(ftp::Client& client, const std::string& argStr, const Args& args) const;
  
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
