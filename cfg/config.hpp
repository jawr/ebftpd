#ifndef __CFG_CONFIG_CPP
#define __CFG_CONFIG_CPP
#include <vector>
#include <string>
#include <map>
#include "cfg/exception.hpp"
#include "cfg/setting.hpp"

namespace cfg
{

class Config
{ 
  std::string config;
  std::map<std::string, std::vector<Setting> > settings;
public:
  Config(const std::string& configFile);
  ~Config() {};  

  void Parse(const std::string& line);
  Setting *GetSetting(const std::string& setting, 
    std::vector<std::string>& toks);

};

}

#endif 
