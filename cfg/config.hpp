#ifndef __CFG_CONFIG_CPP
#define __CFG_CONFIG_CPP
#include <vector>
#include <string>
#include <boost/unordered_map.hpp>
#include <tr1/memory>
#include "cfg/exception.hpp"
#include "cfg/setting.hpp"
#include "fs/path.hpp"


namespace cfg
{

class Config
{ 
  static int latestVersion;
  int version;

  std::string config;
 
  void Parse(const std::string& line);

  // containers

public:
  Config(const std::string& configFile);
  ~Config() {};  

  int Version() const { return version; };

};

typedef std::tr1::shared_ptr<cfg::Config> ConfigPtr;

}


#endif 
