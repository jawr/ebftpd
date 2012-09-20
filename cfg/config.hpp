#ifndef __CFG_CONFIG_CPP
#define __CFG_CONFIG_CPP
#include <vector>
#include <string>
#include <tr1/unordered_map>
#include "cfg/exception.hpp"
#include "cfg/setting.hpp"

namespace cfg
{

class Config
{ 
  std::string config;
  std::tr1::unordered_map<std::string, std::vector<Setting*> > settings;
 
  // wanted to pass a vector reference to this function as well as the
  // associated *Cast function to handle the settings. although it seems i need
  // the defnititon here. guess we could use a template instead.
  // void GetSettingVector( (*f)(Setting*), 
  setting::ACL* ACLCast(Setting* s) { return std::static_cast<setting::ACL*>(s); };
  setting::Path* PathCast(Setting* s) { return std::static_cast<setting::Path*>(s); };
public:
  Config(const std::string& configFile);
  ~Config() {};  

  void Parse(const std::string& line);
  Setting *GetSetting(const std::string& setting, 
    std::vector<std::string>& toks);

  // getters
  // using vector as return seems slightly inefficient.
  // follows the following syntax:
  //  userrejectsecure -> Userrejectsecure
  //  ascii_downloads  -> AsciiDownloads 
  std::vector<setting::Path*> DSACertFile();
  std::vector<setting::ACL*> Userrejectsecure();
  std::vector<setting::ACL*> Userrejectinsecure();
  std::vector<setting::ACL*> Denydiruncrypted();
  std::vector<setting::ACL*> Denydatauncrypted();

};

}

#endif 
