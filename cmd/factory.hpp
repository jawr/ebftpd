#ifndef __CMD_FACTORY_HPP
#define __CMD_FACTORY_HPP

#include <tr1/memory>
#include <tr1/unordered_map>
#include "cmd/command.hpp"

namespace ftp
{
class Client; 
}

namespace cmd
{

template <class BaseT>
class CreatorBase
{
public:  
  virtual BaseT *Create(ftp::Client& client, const Args& args) = 0;
};

template <class CommandT>
class Creator : public CreatorBase<Command>
{
public:  
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
  static Command* Create(ftp::Client& client, const Args& args);
};


} /* cmd namespace */

#endif
