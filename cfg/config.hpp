#ifndef __CFG_CONFIG_CPP
#define __CFG_CONFIG_CPP
#include <vector>
#include <array>
#include <string>
#include "../acl/common.hpp"
#include "../util/endpoint.hpp"

namespace cfg 
{
class StatSection
{
  std::string keyword;
  std::string directory;
  bool seperateCredits;
public:
  StatSection(const std::string& keyword, const std::string& directory, 
    bool seperateCredits) : 
    keyword(keyword), directory(directory), seperateCredits(seperateCredits) {};
  ~StatSection();
};

class Right
{
  std::string function; // could be enum
  std::string path;
  acl::ACL acl;
public:
  Right(const std::string& function, const std::string& path, 
    const std::string& acl);
  ~Right();
};

class Creditloss
{
  int multiplier;
  bool allowLeechers;
  std::string path;
public:
  Creditloss(int multiplier, bool allowLeechers, const std::string& path) :
    multiplier(multiplier), allowLeechers(allowLeechers), path(path) {};
  ~Creditloss();
};
 
class Creditcheck
{
  std::string path;
  int ratio;
  acl::ACL acl;
public:
  Creditcheck(const std::string& path, int ratio, const std::string& acl);
  ~Creditcheck();
};

class Config 
{
  std::string siteNameLong;
  std::string rootPath;
  std::string dataPath;

  std::string loginPrompt;
  std::string welcomeMsg;
  std::string goodbyeMsg;
  std::string newsfile;
  std::string banner;
  
  std::vector<util::tcp::endpoint> activeAddr;
  std::vector<util::tcp::endpoint> pasvAddr;
  std::vector<util::tcp::endpoint> bouncers;
  long pasvPortsStart, pasvPortsEnd;

  std::vector<std::string> bannedUsers;

  std::vector<StatSection> statSections;
  std::vector<Right> rights;
  std::vector<Creditloss> creditloss;
  std::vector<Creditcheck> creditcheck;


public:
  Config();
  ~Config();  
};

}

#endif 
