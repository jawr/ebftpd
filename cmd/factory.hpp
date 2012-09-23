#ifndef __CMD_FACTORY_HPP
#define __CMD_FACTORY_HPP

#include <tr1/memory>
#include <tr1/unordered_map>
#include "cmd/command.hpp"
#include "ftp/client.hpp"

namespace cmd
{

template <class BaseT>
class CreatorBase
{
  ftp::ClientState requiredState;
  
public:  
  CreatorBase(ftp::ClientState requiredState) :
    requiredState(requiredState) { }
  virtual BaseT *Create(ftp::Client& client, const Args& args) = 0;
  ftp::ClientState RequiredState() const { return requiredState; }
};

template <class CommandT>
class Creator : public CreatorBase<Command>
{
public:
  Creator(ftp::ClientState requiredState) : CreatorBase(requiredState) { }
  Command *Create(ftp::Client& client, const Args& args)
  {
    return new CommandT(client, args);
  }
};

class Factory
{
  typedef std::tr1::unordered_map<std::string,
                  CreatorBase<Command>* > CreatorsMap;
                                   
  CreatorsMap creators;
   
  Factory();
  ~Factory();
  
  void Register(const std::string& command, CreatorBase<Command>* creator);
  
  static Factory factory;
  
public:
  static Command* Create(ftp::Client& client, const Args& args,
                         ftp::ClientState& requiredState);
};

} /* cmd namespace */

#endif
