#include <iostream>
#include <fstream>
#include <string>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "config.hpp"
#include "exception.hpp"
#include "acl/common.hpp"

namespace cfg
{

Config::Config(const std::string& config) : config(config), settings()
{
  std::string line;
  std::ifstream io(config.c_str(), std::ifstream::in);

  if (!io.is_open()) throw ConfigFileError();

  while (io.good())
  {
    std::getline(io, line);
    if (line.size() == 0) continue;
    if (line.size() > 0 && line.at(0) == '#') continue;
    Parse(line);
  }
}

void Config::Parse(const std::string& line) {
  std::vector<std::string> toks;
  boost::split(toks, line, boost::is_any_of("\t "));
  if (toks.size() == 0) return;
  if (toks.at(0).size() == 0) return;
  std::string& setting = toks.at(0);
  std::cout << setting << std::endl;
  // remove setting from args
  toks.erase(toks.begin());

  // parse string
  
}

*Setting GetSetting(const std::string& line, std::vector<std::string>& toks)
{
  if (line == "DSA_CERT_FILE")
    return new ::setting::Path(toks.at(0));
  else if (line == "shutdown")
    reutrn new ::setting::Argument(toks.at(0));
  else if (line == "xdupe")
    return new ::setting::Arguments(toks);
  else if (line == "sitename_long")
    return new ::setting::Argument(toks.at(0));
  else if (line == "sitename_short")
    return new ::setting::Argument(toks.at(0));
  else if  (line == "email")
    return new ::setting::Argument(toks.at(0));
  else if (line == "login_prompt")
    return new ::setting::Argument(toks.at(0));
  else if (line == "rootpath")
    return new ::setting::Path(toks.at(0));
  else if (line == "datapath")
    return new ::setting::Path(toks.at(0));
  // was going to store these as endpoints but we have no ports for them
  // thoughts?
  else if (line == "bouncer_ip")
    return new ::setting::Path(toks);
  else if (line == "welcome_msg")
    return new ::setting::ACLWithPath(toks.at(0), toks.at(1));
  else if (line == "goodbye_msg")
    return new ::setting::ACLWithPath(toks.at(0), toks.at(1));
  else if (line == "newsfile")
    return new ::setting::ACLWithPath(toks.at(0), toks.at(1));
  else if (line == "banner")
    return new ::setting::Path(toks.at(0));
  else if (line == "userrejectsecure")
    return new ::setting::ACL(toks.at(0));
  else if (line == "userrejectinsecure")
    return new ::setting::ACL(toks.at(0));
  else if (line == "denydiruncrypted")
    return new ::setting::ACL(toks.at(0));
  else if (line == "denydatauncrypted")
    return new ::setting::ACL(toks.at(0));
  else if (line == "color_mode")
    return new ::setting::Bool(boost::lexical_cast<bool>(toks.at(0)));
  // again, I was initialy planning to store these as endpoints
  else if (line == "active_addr")
    return new ::setting::Argument(toks.at(0));
  else if (line == "pasv_addr")
    return new ::setting::Argument(toks.at(0));

  else if (line == "pasv_ports")
  {
    std::vector<std::string> range;
    boost::split(range, toks.at(0), "-");
    return new ::setting::Range(int(range.at(0)), int(range.at(1)));
  }

  else if (line == "speed_limit")
    return new ::setting::SpeedLimit(toks.at(0), int(toks.at(1)),
      int(toks.at(2)), toks.at(3));


}
