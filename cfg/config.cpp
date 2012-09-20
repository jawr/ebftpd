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

Config::Config(const std::string& config) : config(config), 
MapPath(), MapACL(), MapIntWithArguments(), MapStrings(), MapInt(), MapBool(),
MapSecureIP(), MapSpeedLimit(), MapPasvAddr(), MapAllowFxp(), MapACLWithPath(),
MapPathWithArgument(), MapACLWithArgument(), MapStatSection(), MapPathFilter(),
MapACLWithInt(), MapIntWithBool(), MapRequests(), MapCreditcheck(), 
MapNukedirStyle(), MapMsgPath(), MapSiteCmd(), MapCscript()
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
  SetSetting(opt, toks);
  // push onto setting's vector
  boost::algorithm::to_lower(opt);
  
}

void Config::SetSetting(const std::string& opt, std::vector<std::string>& toks)
{
  if (opt == "DSA_CERT_FILE")
  {
    //return new setting::Path(toks.at(0));
  }
  else if (opt == "userrejectsecure")
  {
    //return new setting::ACL(toks);
  }
  else if (opt == "userrejectinsecure")
  {
    //return new setting::ACL(toks);
  }
  else if (opt == "denydiruncrypted")
  {
    //return new setting::ACL(toks);
  }
  else if (opt == "denydatauncrypted")
  {
    //return new setting::ACL(toks);
  }
  // http://www.glftpd.dk/files/docs/glftpd.docs
  else if (opt == "ascii_downloads")
  {
    //return new setting::IntWithArgs(
    //  boost::lexical_cast<int>(toks.at(0)), toks);
  }
  else if (opt == "shutdown")
  {
    //return new setting::ACL(toks);
  }
  else if (opt == "free_space")
  {
    //return new setting::Int(boost::lexical_cast<int>(toks.at(0)));
  }
  else if (opt == "use_dir_size")
  {
    //return new setting::Arguments(toks);
  }
  else if (opt == "timezone")
  {
    //return new setting::Argument(toks.at(0));
  }
  else if (opt == "color_mode")
  {
    //return new setting::Bool(util::string::BoolLexicalCast(toks.at(0)));
  }
  else if (opt == "sitename_long")
  {
    //return new setting::Argument(toks.at(0));
  }
  else if (opt == "sitename_short")
  {
    //return new setting::Argument(toks.at(0));
  }
  else if (opt == "login_prompt")
  {
    //return new setting::Argument(toks.at(0));
  }
  else if (opt == "rootpath")
  {
    //return new setting::Path(toks.at(0));
  }
  else if (opt == "datapath")
  {
    //return new setting::Path(toks.at(0));
  }
  else if (opt == "reload_config")
  {
    //return new setting::Path(toks.at(0));
  }
  else if (opt == "master")
  {
    //return new setting::Arguments(toks);
  }
  else if (opt == "secure_ip")
  {
    //return new setting::SecureIP(
    //  boost::lexical_cast<int>(toks.at(0)),
    //  util::string::BoolLexicalCast(toks.at(1)),
    //  util::string::BoolLexicalCast(toks.at(2)),
    //  toks.at(3));
  }
  else if (opt == "secure_pass") 
  {
    std::string path = toks.at(0);
    toks.erase(toks.begin());
    //return new setting::ACLWithPath(path, toks);
  }
  else if (opt == "datapath")
  {
    //return new setting::Path(toks.at(0));
  }
  else if (opt == "pwd_path")
  {
    //return new setting::Path(toks.at(0));
  }
  else if (opt == "grp_path")
  {
    //return new setting::Path(toks.at(0));
  }
  else if (opt == "botscript_path")
  {
    //return new setting::Path(toks.at(0));
  }
  else if (opt == "bouncer_ip")
  {
    //return new setting::Arguments(toks);
  }
  else if (opt == "speed_limit")
  {
    //return new setting::SpeedLimit(toks.at(0), 
      //boost::lexical_cast<int>(toks.at(1)), 
      //boost::lexical_cast<int>(toks.at(2)), toks.at(3));
  }
  else if (opt =="sim_xfers")
  {
    std::vector<int> ints;
    std::vector<std::string>::iterator it;
    for (it = toks.begin(); it != toks.end(); ++it)
      ints.push_back(boost::lexical_cast<int>(*it));
    //return new setting::Ints(ints);
  }
  else if (opt == "calc_crc")
  {
    //return new setting::Path(toks.at(0));
  }
  else if (opt == "xdupe")
  {
    //return new setting::Arguments(toks);
  }
  else if (opt == "mmap_amount")
  {
    //return new setting::Int(boost::lexical_cast<int>(toks.at(0)));
  }
  else if (opt == "dl_sendfile")
  {
    //return new setting::Int(boost::lexical_cast<int>(toks.at(0)));
  }
  else if (opt == "ul_buffered_force")
  {
    //return new setting::Int(boost::lexical_cast<int>(toks.at(0)));
  }
  else if (opt == "min_homedir")
  {
    //return new setting::Path(toks.at(0));
  }
  else if (opt == "valid_ip")
  {
    //return new setting::Arguments(toks);
  }
  else if (opt == "active_addr")
  {
    //return new setting::Argument(toks.at(0));
  }
  else if (opt == "pasv_addr") 
  {
    if (toks.size() == 1)
    {
      //return new setting::PasvAddr(toks.at(0));
    }
    else
    {
      //return new setting::PasvAddr(toks.at(0), 
      //  util::string::BoolLexicalCast(toks.at(1)));
    }
  }
  // instead of transforming to ints keep as strings:
  // Exmp: pasv_ports 10000-11000 20 21 22 23 80 110 1600-1610 35000-35050
  else if (opt == "pasv_ports")
  {
    //return new setting::Arguments(toks);
  }
  else if (opt == "active_ports")
  {
    //return new setting::Arguments(toks);
  }
  else if (opt =="allow_fxp")
  {
    //return new setting::AllowFXP(
    //  util::string::BoolLexicalCast(toks.at(0)),
    //  util::string::BoolLexicalCast(toks.at(1)),
    //  util::string::BoolLexicalCast(toks.at(2)),
     // toks.at(3));
  }
  else if (opt == "welcome_msg" || opt == "goodbye_msg" || opt == "newsfile")
  {
    std::string path = toks.at(0);
    toks.erase(toks.begin());
    //return new setting::ACLWithPath(path, toks);
  }
  else if (opt == "banner")
  {
    //return new setting::Path(toks.at(0));
  }
  else if (opt == "alais")
  {
    //return new setting::PathWithArgument(toks.at(1), toks.at(0));
  }
  else if (opt == "cdpath")
  {
    //return new setting::Argument(toks.at(0));
  }
  else if (opt == "ignore_type")
  {
    //return new setting::Arguments(toks);
  }
  // rights section
  else if (opt == "delete" || opt == "deleteown" || opt == "overwrite" 
    || opt == "resume" || opt == "rename" || opt == "renameown" 
    || opt == "filemove" || opt == "makedir" || opt == "upload"
    || opt == "download" || opt == "nuke" || opt == "dirlog"
    || opt == "hideinwho" || opt == "nostats")
  {
    std::string path = toks.at(0);
    toks.erase(toks.begin());
    //return new setting::ACLWithPath(path, toks);
  }
  else if (opt == "freefile") 
  {
    std::string argument = toks.at(0);
    toks.erase(toks.begin());
    //return new setting::ACLWithArgument(argument, toks);
  }
  // stats section
  else if (opt == "stat_section")
  {
    //return new setting::StatSection(toks.at(0), toks.at(1), 
     // util::string::BoolLexicalCast(toks.at(2)));
  }
  else if (opt == "path-filter")
  {
    std::string group = toks.at(0);
    std::string messageFile = toks.at(1);
    toks.erase(toks.begin(), toks.begin()+2);
    //return new setting::PathFilter(group, messageFile, toks);
  }
  else if (opt == "max_users")
  {
    std::vector<int> ints;
    std::vector<std::string>::iterator it;
    for (it = toks.begin(); it != toks.end(); ++it)
      ints.push_back(boost::lexical_cast<int>(*it));
    //return new setting::Ints(ints);
  }
  else if (opt == "max_ustats" || opt == "max_gstats")
  {
    int num = boost::lexical_cast<int>(toks.at(0));
    toks.erase(toks.begin());
    //return new setting::ACLWithInt(num, toks);
  }
  else if (opt == "banned_users")
  {
    //return new setting::Arguments(toks);
  }
  else if (opt == "show_diz")
  {
    std::string path = toks.at(0);
    toks.erase(toks.begin());
    if (toks.size() == 0) toks.push_back(std::string("*"));
    //return new setting::ACLWithPath(path, toks);
  }
  else if (opt == "show_totals")
  {
    int maxLines = (toks.at(0) == "*") ? -1 : boost::lexical_cast<int>(toks.at(0));
    //return new setting::IntWithArgs(maxLines, toks);
  }
  else if (opt == "dl_incomplete")
  {
    //return new setting::Bool(util::string::BoolLexicalCast(toks.at(0)));
  }
  else if (opt == "fild_dl_count")
  {
    //return new setting::Bool(util::string::BoolLexicalCast(toks.at(0)));
  }
  else if (opt == "dupe_check")
  {
    //return new setting::IntWithBool(boost::lexical_cast<int>(toks.at(0)),
    //  util::string::BoolLexicalCast(toks.at(1)));
  }
  // built in scripts
  else if (opt == "pre_check" || opt == "pre_dir_check" || opt == "post_check")
  {
    std::string path = toks.at(0);
    toks.erase(toks.begin());
    //return new setting::ACLWithPath(path, toks);
  }
  else if (opt == "idle_commands")
  {
    //return new setting::Arguments(toks);
  }
  else if (opt == "total_users")
  {
    //return new setting::Int(boost::lexical_cast<int>(toks.at(0)));
  }
  else if (opt == "lslong")
  {
    //return new setting::Arguments(toks);
  }
  else if (opt == "hidden_files")
  {
    //return new setting::Arguments(toks);
  }
  else if (opt == "noretrieve")
  {
    //return new setting::Arguments(toks);
  }
  else if (opt == "dir_names" || opt == "file_names")
  {
    int caps = util::string::BoolLexicalCast(toks.at(0));
    std::string toLower = toks.at(1);
    boost::algorithm::to_lower(toLower);
    bool upperCase = (toLower == "lower") ?
      false : true;
    toks.erase(toks.begin(), toks.begin()+2);
    //return new setting::NameRules(caps, upperCase, toks);
  }
  else if (opt == "tagline")
  {
    //return new setting::Argument(toks.at(0));
  }
  else if (opt == "email")
  {
    //return new setting::Argument(toks.at(0));
  }
  else if (opt == "multiplier_max" || opt == "oneliners")
  {
    //return new setting::Int(boost::lexical_cast<int>(toks.at(0)));
  }
  else if (opt == "requests")
  {
    //return new setting::Requests(toks.at(0), 
      //boost::lexical_cast<int>(toks.at(1)));
  }
  else if (opt == "lastonline")
  {
    std::vector<int> ints;
    std::vector<std::string>::iterator it;
    for (it = toks.begin(); it != toks.end(); ++it)
      ints.push_back(boost::lexical_cast<int>(*it));
    //return new setting::Ints(ints);
  }
  else if (opt == "empty_nuke")
  {
    //return new setting::Int(boost::lexical_cast<int>(toks.at(0)));
  }
  else if (opt == "nodupecheck")
  {
    //return new setting::Path(toks.at(0));
  }
  else if (opt == "creditcheck")
  {
    std::string path = toks.at(0);
    int ratio = boost::lexical_cast<int>(toks.at(1));
    toks.erase(toks.begin(), toks.begin()+2);
    //return new setting::CreditCheck(path, ratio, toks);
  }
  else if (opt == "creditloss")
  {
    int multiplier = boost::lexical_cast<int>(toks.at(0));
    bool leechers = util::string::BoolLexicalCast(toks.at(1));
    std::string path = toks.at(2);
    toks.erase(toks.begin(), toks.begin()+3);
    //return new setting::CreditLoss(multiplier, leechers, path, toks);
  }
  else if (opt == "nukedir_style")
  {
    //return new setting::NukeDirStyle(toks.at(0), 
    //  boost::lexical_cast<int>(toks.at(1)),
    //  boost::lexical_cast<int>(toks.at(2)));
  }
  else if (opt == "hideuser")
  {
    //return new setting::Arguments(toks);
  }
  else if (opt == "privgroup")
  {
    //return new setting::Arguments(toks);
  }
  else if (opt == "msgpath")
  {
    std::string path = toks.at(0);
    std::string filename = toks.at(1);
    toks.erase(toks.begin(), toks.begin()+2);
    //return new setting::MsgPath(path, filename, toks);
  }
  else if (opt == "privpath")
  {
    std::string path = toks.at(0);
    toks.erase(toks.begin());
    //return new setting::ACLWithPath(path, toks);
  }
  else if (opt == "max_sitecmd_lines")
  {
    //return new setting::Int(boost::lexical_cast<int>(toks.at(0)));
  }
  else if (opt == "csript")
  {
    //return new setting::CScript(toks.at(0), toks.at(1), toks.at(2)); 
  }
    
  // todo: 
  //  site_cmd and their corresponding permissions
  
  throw NoSetting("Unable to find setting '" + opt + "'");
}

}

#ifdef CONFIG_TEST
int main()
{
  cfg::Config("glftpd.conf");
  return 0;
}
#endif
  
