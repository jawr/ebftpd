#ifndef __CFG_FACTOTY_HPP
#define __CFG_FACTORY_HPP
#include <string>
#include <tr1/unordered_map>
#include "cfg/setting.hpp"
namespace cfg
{

template <class T>
class CreatorBase
{
public:
  virtual ~CreatorBase() {};
  virtual T *Create() = 0;
};

template <class T>
class Creator : public CreatorBase<setting::Setting>
{
public:
  Creator() {};
  setting::Setting *Create() { return new T; };
};

class Factory
{
  std::tr1::unordered_map<std::string, 
    CreatorBase<setting::Setting*> > registry;
public:
  Factory();
  setting::Setting *Create(const std::string& name);
  void Register(const std::string& name, CreatorBase<setting::Setting>* creator)
  {
    registry.insert(std::make_pair(name, creator));
  };
};
// end namespace
}
#endif
