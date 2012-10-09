#ifndef __CMD_FACTORY_HPP
#define __CMD_FACTORY_HPP

#include <memory>
#include <tr1/unordered_map>
#include "cmd/command.hpp"
#include "ftp/client.hpp"

namespace cmd
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
class Creator : public CreatorBase<Command>
{
public:
  Creator(ftp::ClientState reqdState) : CreatorBase(reqdState) { }
  Command *Create(ftp::Client& client, const std::string& argStr,
                  const Args& args)
  {
    return new CommandT(client, argStr, args);
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
  static Command* Create(ftp::Client& client, const std::string& argStr,
                         const Args& args, ftp::ClientState& reqdState);
};

} /* cmd namespace */

#endif
