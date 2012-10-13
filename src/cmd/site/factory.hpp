#ifndef __CMD_SITE_FACTORY_HPP
#define __CMD_SITE_FACTORY_HPP

#include <memory>
#include <tr1/unordered_map>
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

class Factory
{
  typedef std::tr1::unordered_map<std::string,
                  CreatorBase<cmd::Command>* > CreatorsMap;
                                   
  CreatorsMap creators;
   
  Factory();
  ~Factory();
  
  void Register(const std::string& command, CreatorBase<cmd::Command>* creator);
  
  static Factory factory;
  
public:
  static cmd::Command* Create(ftp::Client& client, const std::string& argStr,
                         const Args& args);
};

} /* site namespace */
} /* cmd namespace */

#endif
