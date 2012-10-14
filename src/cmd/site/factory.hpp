#ifndef __CMD_SITE_FACTORY_HPP
#define __CMD_SITE_FACTORY_HPP

#include <memory>
#include <unordered_map>
#include "cmd/command.hpp"
#include "ftp/client.hpp"

namespace cmd { namespace site
{

template <class BaseT>
class CreatorBase
{
  std::string aclKeyword;
public:  
  CreatorBase(const std::string& aclKeyword) :
    aclKeyword(aclKeyword) { }
  virtual ~CreatorBase() { }
  
  virtual BaseT *Create(ftp::Client& client, const std::string& argStr,
                        const Args& args) = 0;
  const std::string& ACLKeyword() const { return aclKeyword; }
};

template <class CommandT>
class Creator : public CreatorBase<cmd::Command>
{
public:
  Creator(const std::string& aclKeyword = "") :
    CreatorBase(aclKeyword) { }
  cmd::Command *Create(ftp::Client& client, const std::string& argStr,
                  const Args& args)
  {
    return new CommandT(client, argStr, args);
  }
};

class Factory
{
  typedef std::unordered_map<std::string,
                  CreatorBase<cmd::Command>* > CreatorsMap;
                                   
  CreatorsMap creators;
   
  Factory();
  ~Factory();
  
  void Register(const std::string& command, CreatorBase<cmd::Command>* creator);
  
  static Factory factory;
  
public:
  static cmd::Command* Create(ftp::Client& client, const std::string& argStr,
                         const Args& args, std::string& aclKeyword);
};

} /* site namespace */
} /* cmd namespace */

#endif
