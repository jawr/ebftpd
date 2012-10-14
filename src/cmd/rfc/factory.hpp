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
  ftp::ClientState reqdState;
  
public:  
  CreatorBase(ftp::ClientState reqdState) : reqdState(reqdState) { }
  virtual ~CreatorBase() { }
  
  virtual BaseT *Create(ftp::Client& client, const std::string& argStr,
                        const Args& args) = 0;
  ftp::ClientState ReqdState() const { return reqdState; }
};

template <class CommandT>
class Creator : public CreatorBase<cmd::Command>
{
public:
  Creator(ftp::ClientState reqdState) : CreatorBase(reqdState) { }
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
                              const Args& args, ftp::ClientState& reqdState);
};

} /* rfc namespace */
} /* cmd namespace */

#endif
