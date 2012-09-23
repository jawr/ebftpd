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
  SetSetting(opt, toks);
  // push onto setting's vector
  
}

void Config::SetSetting(const std::string& opt, std::vector<std::string>& toks)
{

  // http://www.glftpd.dk/files/docs/glftpd.docs

  // Path
  if (opt == "dsa_cert_file" || opt == "rootpath" || opt == "datapath"
   || opt == "pwd_path" || opt == "grp_path" || opt == "botscript_path"
   || opt == "calc_crc" || opt == "min_homedir" || opt == "banner"
   || opt == "nodupecheck" || opt == "rsa_cert_file")
  {
      fs::Path path(toks.at(0));
      InsertSetting<fs::Path>(MapPath, path, opt);
  }

  // ACL 
  else if (opt == "userrejectsecure" || opt == "userrejectinsecure" 
   || opt == "denydiruncrypted" || opt == "denydatauncrypted"
   || opt == "shutdown" || opt == "hideuser")
  {
    setting::ACL acl(toks);
    InsertSetting<setting::ACL>(MapACL, acl, opt);  
  }

  // IntWithArguments 
  else if (opt == "ascii_downloads" || opt == "show_totals")
  {
    int i = (toks.at(0) == "*") ? -1 : boost::lexical_cast<int>(toks.at(0));
    toks.erase(toks.begin());
    setting::IntWithArguments set(i, toks);
    InsertSetting<setting::IntWithArguments>(MapIntWithArguments, set, opt);
  }

  // int  
  else if (opt == "free_space" || opt == "timezone"
   || opt == "mmap_amount" || opt == "dl_sendfile" || opt == "ul_buffered_force"
   || opt == "total_users" 
   || opt == "empty_nuke" || opt == "max_sitecmd_lines" || opt == "oneliners"
   || opt == "multiplier_max")
  {
    InsertSetting<int>(MapInt, boost::lexical_cast<int>(toks.at(0)), opt);
  }

  // vectorint
  else if (opt == "sim_xfers" || opt == "max_users" || opt == "lastonline")
  {
    std::vector<int> set;
    for (std::vector<std::string>::iterator it = toks.begin(); it != toks.end();
      ++it)
      set.push_back(boost::lexical_cast<int>(*it)); 
    InsertSetting<std::vector<int> >(MapVectorInt, set, opt);
  }

  // Strings
  else if (opt == "use_dir_size" || opt == "sitename_long" || opt == "sitename_short"
   || opt == "login_prompt" || opt == "tagline" || opt == "email" || opt == "cdpath")
  {
    InsertSetting<std::string>(MapString, toks.at(0), opt);
  }

  // VectorStrings
  else if (opt == "master" || opt == "bouncer_ip" 
   || opt == "xdupe" || opt == "valid_ip" || opt == "active_addr"
   || opt == "pasv_ports" || opt == "active_ports"
   || opt == "ignore_type" || opt == "banned_users" || opt == "idle_commands"
   || opt == "lslong" || opt == "hidden_files" || opt == "noretrieve" 
   || opt == "privgroup")
  {
    InsertSetting<std::vector<std::string> >(MapVectorString, toks, opt);
  }

  // Bool
  else if (opt == "color_mode" || opt == "dl_incomplete" 
   || opt == "file_dl_count")
  {
    InsertSetting<bool>(MapBool, util::string::BoolLexicalCast(toks.at(0)), opt); 
  }

  // SecureIP
  else if (opt == "secure_ip")
  {
    int fields = boost::lexical_cast<int>(toks.at(0));
    bool allowHostnames = util::string::BoolLexicalCast(toks.at(1));
    bool needIdent = util::string::BoolLexicalCast(toks.at(2));
    toks.erase(toks.begin(), toks.begin()+3);
    setting::SecureIP set(fields, allowHostnames, needIdent, toks);
    InsertSetting<setting::SecureIP>(MapSecureIP, set, opt);
  }

  // ACLWithPath
  else if (opt == "secure_pass" || opt == "welcome_msg" || opt == "goodbye_msg"
   || opt == "newsfile" || opt == "delete" || opt == "deleteown" 
   || opt == "overwrite" || opt == "resume" || opt == "rename" 
   || opt == "renameown" || opt == "filemove" || opt == "makedir" 
   || opt == "upload" || opt == "download" || opt == "nuke" || opt == "dirlog"
   || opt == "hideinwho" || opt == "nostats" || opt == "show_diz"
   || opt == "pre_check" || opt == "pre_dir_check" || opt == "post_check"
   || opt == "privpath")
  {
    std::string path = toks.at(0);
    toks.erase(toks.begin());
    setting::ACLWithPath set(path, toks);
    InsertSetting<setting::ACLWithPath>(MapACLWithPath, set, opt);
  }

  // SpeedLimit
  else if (opt == "speed_limit")
  {
    std::string path = toks.at(0);
    int upload = boost::lexical_cast<int>(toks.at(1));
    int download = boost::lexical_cast<int>(toks.at(2));
    toks.erase(toks.begin(), toks.begin()+3);
    setting::SpeedLimit set(path, upload, download, toks);
    InsertSetting<setting::SpeedLimit>(MapSpeedLimit, set, opt); 
  }

  // PasvAddr
  else if (opt == "pasv_addr") 
  {
    setting::PasvAddr set(toks.at(0));
    if (toks.size() > 1)
      set.SetPrimary();
    InsertSetting<setting::PasvAddr>(MapPasvAddr, set, opt);
  }

  // AllowFxp
  else if (opt =="allow_fxp")
  {
    bool downloads = util::string::BoolLexicalCast(toks.at(0));
    bool uploads   = util::string::BoolLexicalCast(toks.at(1)); 
    bool logging   = util::string::BoolLexicalCast(toks.at(2));
    toks.erase(toks.begin(), toks.begin()+3);
    setting::AllowFxp set(downloads, uploads, logging, toks);
    InsertSetting<setting::AllowFxp>(MapAllowFxp, set, opt);
  }

  // PathWithArgument
  else if (opt == "alais")
  {
    setting::PathWithArgument set(toks.at(0), toks.at(1));
    InsertSetting<setting::PathWithArgument>(MapPathWithArgument, set, opt);
  }

  // ACLWithArgument
  else if (opt == "freefile") 
  {
    std::string argument = toks.at(0);
    toks.erase(toks.begin());
    setting::ACLWithArgument set(argument, toks);
    InsertSetting<setting::ACLWithArgument>(MapACLWithArgument, set, opt);
  }

  // StatSection
  else if (opt == "stat_section")
  {
    setting::StatSection set(toks.at(0), toks.at(1), 
     util::string::BoolLexicalCast(toks.at(2)));
    InsertSetting<setting::StatSection>(MapStatSection, set, opt);
  }

  // PathFilter
  else if (opt == "path-filter")
  {
    std::string group = toks.at(0);
    std::string messageFile = toks.at(1);
    toks.erase(toks.begin(), toks.begin()+2);
    setting::PathFilter set(group, messageFile, toks);
    InsertSetting<setting::PathFilter>(MapPathFilter, set, opt);
  }

  // ACLWithInt
  else if (opt == "max_ustats" || opt == "max_gstats")
  {
    int num = boost::lexical_cast<int>(toks.at(0));
    toks.erase(toks.begin());
    setting::ACLWithInt set(num, toks);
    InsertSetting<setting::ACLWithInt>(MapACLWithInt, set, opt);
  }


  // IntWithBool
  else if (opt == "dupe_check")
  {
    int first = boost::lexical_cast<int>(toks.at(0));
    bool enabled = util::string::BoolLexicalCast(toks.at(1));
    setting::IntWithBool set(first, enabled);
    InsertSetting<setting::IntWithBool>(MapIntWithBool, set, opt);
  }

  // Requests
  else if (opt == "requests")
  {
    setting::Requests set(toks.at(0), boost::lexical_cast<int>(toks.at(1)));
    InsertSetting<setting::Requests>(MapRequests, set, opt);
  }

  // Creditcheck
  else if (opt == "creditcheck")
  {
    std::string path = toks.at(0);
    int ratio = boost::lexical_cast<int>(toks.at(1));
    toks.erase(toks.begin(), toks.begin()+2);
    setting::Creditcheck set(path, ratio, toks);
    InsertSetting<setting::Creditcheck>(MapCreditcheck, set, opt);
  }
  
  // Creditloss
  else if (opt == "creditloss")
  {
    int multiplier = boost::lexical_cast<int>(toks.at(0));
    bool leechers = util::string::BoolLexicalCast(toks.at(1));
    std::string path = toks.at(2);
    toks.erase(toks.begin(), toks.begin()+3);
    setting::Creditloss set(multiplier, leechers, path, toks);
    InsertSetting<setting::Creditloss>(MapCreditloss, set, opt);
  }

  // NukedirStyle
  else if (opt == "nukedir_style")
  {
    setting::NukedirStyle set(toks.at(0), 
      boost::lexical_cast<int>(toks.at(1)),
      boost::lexical_cast<int>(toks.at(2)));
    InsertSetting<setting::NukedirStyle>(MapNukedirStyle, set, opt);
  }

  // MsgPath
  else if (opt == "msg_path")
  {
    std::string path = toks.at(0);
    std::string filename = toks.at(1);
    toks.erase(toks.begin(), toks.begin()+2);
    setting::MsgPath set(path, filename, toks);
    InsertSetting<setting::MsgPath>(MapMsgPath, set, opt);
  }

  // Cscript
  else if (opt == "cscript")
  {
    setting::Cscript set(toks.at(0), toks.at(1), toks.at(2)); 
    InsertSetting<setting::Cscript>(MapCscript, set, opt);
  }

  // SiteCmd
  else if (opt == "site_cmd")
  {
    std::string command = toks.at(0);
    std::string method = toks.at(1);
    boost::algorithm::to_lower(method);
    setting::SiteCmdMethod method_ = setting::EXEC;
    if (method == "text") method_  = setting::TEXT;
    else if (method == "is") method_ = setting::IS;
    std::string path = toks.at(2);
    toks.erase(toks.begin(), toks.begin()+3);

    setting::SiteCmd set(command, method_, path, toks);
    InsertSetting<setting::SiteCmd>(MapSiteCmd, set, opt);
  }

  // check if we have a site_cmd permission
  else if (opt.at(0) == '-')
  {
    std::string opt_ = opt;
    opt_.erase(opt_.begin());
    
  } 
  else if (opt.find("custom-") != std::string::npos)
  {
    std::string opt_ = opt;
    opt_.replace(0, 7, "");
  }
   
  else
    throw NoSetting("Unable to find setting '" + opt + "'");
}

void Config::SetDefaults()
{

}

}

#ifdef CFG_CONFIG_TEST
int main()
{
  cfg::Config c("glftpd.conf");
  logger::ftpd << c.DSACertFile() << logger::endl; 
  const std::vector<cfg::setting::ACLWithPath>& downloads = c.Download();
  logger::ftpd << "download:" << logger::endl;
  for (std::vector<cfg::setting::ACLWithPath>::const_iterator it = downloads.begin();
    it != downloads.end(); ++it)
    logger::ftpd << (*it).Path() << logger::endl;
  return 0;
}
#endif
  
