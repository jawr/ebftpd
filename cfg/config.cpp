#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "cfg/config.hpp"
#include "cfg/exception.hpp"
#include "cfg/setting.hpp"
#include "util/string.hpp"
#include "logger/logger.hpp"

namespace cfg
{

int Config::latestVersion = 0;

Config::Config(const std::string& config) : version(++latestVersion), config(config)
{
  std::string line;
  std::ifstream io(config.c_str(), std::ifstream::in);
  int i = 0;

  if (!io.is_open()) throw ConfigFileError();

  while (io.good())
  {
    std::getline(io, line);
    ++i;
    if (line.size() == 0) continue;
    if (line.size() > 0 && line.at(0) == '#') continue;
    try 
    {
      Parse(line);
    } 
    catch (NoSetting &e) // handle properly
    {
      ::logger::ftpd << e.what() << " (" << config << ":" << i << ")" << logger::endl;
    }
    catch (...)
    {
      logger::ftpd << "super error on line " << i << logger::endl;
      throw;
    }
  }
}

void Config::Parse(const std::string& line) {
  std::vector<std::string> toks;
  boost::split(toks, line, boost::is_any_of("\t "));
  if (toks.size() == 0) return;
  std::string opt = toks.at(0);
  if (opt.size() == 0) return;
  // remove setting from args
  toks.erase(toks.begin());
  std::vector<std::string>::iterator it;
  for (it = toks.begin(); it != toks.end();)
    if ((*it).size() == 0)
      it = toks.erase(it);
    else
      ++it;
    

  // parse string
  boost::algorithm::to_lower(opt);
  //SetSetting(opt, toks);
  // push onto setting's vector
  
}


}

#ifdef CFG_CONFIG_TEST
int main()
{
  cfg::Config c("glftpd.conf");
  return 0;
}
#endif
  
