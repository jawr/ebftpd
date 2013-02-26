#include <iostream>
#include <sstream>
#include <mongo/client/dbclient.h>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include "util/passwd.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "cfg/error.hpp"
#include "version.hpp"
#include "stats/types.hpp"
#include "util/string.hpp"
#include "stats/compile.hpp"
#include "text/error.hpp"
#include "text/parser.hpp"

namespace po = boost::program_options;

std::shared_ptr<cfg::Config> config;

enum class Who
{
  Users,
  Groups
};

void DisplayHelp(char* argv0, po::options_description& desc)
{
  std::cout << "usage: " << argv0 << " [options]" << std::endl;
  std::cout << desc;
}

void DisplayVersion()
{
  std::cout << "ebftpd ranks " + std::string(version) << std::endl;
}

std::istream& operator>>(std::istream& is, stats::Timeframe& tf)
{
  std::string token;
  is >> token;
  if (!util::EnumFromString(token, tf))
  {
    throw po::validation_error(po::validation_error::invalid_option_value);
  }
  return is;
}

std::istream& operator>>(std::istream& is, stats::Direction& dir)
{
  std::string token;
  is >> token;
  if (!util::EnumFromString(token, dir))
  {
    throw po::validation_error(po::validation_error::invalid_option_value);
  }
  return is;
}

std::istream& operator>>(std::istream& is, stats::SortField& sf)
{
  std::string token;
  is >> token;
  if (!util::EnumFromString(token, sf))
  {
    throw po::validation_error(po::validation_error::invalid_option_value);
  }
  return is;
}

std::ostream& operator<<(std::ostream& os, const stats::SortField& sf)
{
  return (os << util::EnumToString(sf));
}

std::istream& operator>>(std::istream& is, Who& who)
{
  std::string token;
  is >> token;
  util::ToLower(token);
  if (token == "users") who = Who::Users;
  else if (token == "groups") who = Who::Groups;
  else
  {
    throw po::validation_error(po::validation_error::invalid_option_value);
  }
  return is;
}

bool ParseOptions(int argc, char** argv, std::string& configPath, 
                  stats::Timeframe& tf, stats::Direction& dir,
                  stats::SortField& sf, Who& who,
                  std::string& section, bool& raw, int& max,
                  std::string& templatePath)
{
  po::options_description visible("supported options");
  visible.add_options()
    ("help,h", "display this help message")
    ("version,v", "display version")
    ("config-path,c", po::value<std::string>(&configPath), "specify location of config file")
    ("timeframe,t", po::value<stats::Timeframe>(&tf)->required(), "day, week, month, year, alltime")
    ("direction,d", po::value<stats::Direction>(&dir)->required(), "up, down")
    ("sortfield,o", po::value<stats::SortField>(&sf)->default_value(stats::SortField::KBytes, "kbytes"), "kbytes, files, speed")
    ("who,w", po::value<Who>(&who)->required(), "users, groups")
    ("section,s", po::value<std::string>(&section), "stat section, defaults to all if omitted")
    ("raw,r", po::value<bool>(&raw)->default_value(true), "raw formatting")
    ("max,m", po::value<int>(&max)->default_value(10), "maximum entries in output, -1 for unlimited")
    ("template,y", po::value<std::string>(&templatePath), "template file path")
  ;
  
  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(argc, argv).options(visible).run(), vm);

    if (vm.count("help"))
    {
      DisplayHelp(argv[0], visible);
      return false;
    }

    if (vm.count("version"))
    {
      DisplayVersion();
      return false;
    }

    po::notify(vm);
  }
  catch (const po::error& e)
  {
    std::cerr << e.what() << std::endl;
    DisplayHelp(argv[0], visible);
    return false;
  }
  
  return true;
}


int main(int argc, char** argv)
{
  stats::Timeframe tf;
  stats::Direction dir;
  stats::SortField sf;
  std::string section;
  Who who;
  bool raw;
  int max;
  std::string configPath;
  std::string templatePath;

  if (!ParseOptions(argc, argv, configPath, tf, dir, sf, who, 
                    section, raw, max, templatePath))
  {
    return 1;
  }

  try
  {
    config = cfg::Config::Load(configPath, true);
  }
  catch (const cfg::ConfigError& e)
  {
    std::cerr << "Failed to load config: " << e.Message() << std::endl;
    return 1;
  }
  
  cfg::UpdateShared(config);
  
  boost::optional<text::Template> templ;
  if (!templatePath.empty())
  {
    try
    {
      text::TemplateParser parser(templatePath);
      templ.reset(parser.Create());
    }
    catch (const text::TemplateError& e)
    {
      std::cerr << "Unable to load template file: " << e.Message() << std::endl;
      return 1;
    }
  }
  else
  {
    std::string tmplGeneric(cfg::Get().Datapath());
    tmplGeneric += "/text/";
    if (raw) tmplGeneric += "raw";
    
    if (who == Who::Users) tmplGeneric += "ranks";
    else tmplGeneric += "gpranks";
    
    std::string tmplSpecific = tmplGeneric + "." + 
                               util::EnumToString(tf) + "." + 
                               util::EnumToString(dir) + "." + 
                               util::EnumToString(sf);                           
    try
    {
      try
      {
        text::TemplateParser parser(tmplSpecific + ".tmpl");
        templ.reset(parser.Create());
      }
      catch (const text::TemplateError&)
      {
        text::TemplateParser parser(tmplGeneric + ".tmpl");
        templ.reset(parser.Create());
      }
    }
    catch (const text::TemplateError& e)
    {
      std::cerr << "Unable to load template file: " << e.Message() << std::endl;
      return 1;
    }
  }
  
  if (who == Who::Users)
  {
    std::cout << stats::CompileUserRanks(section, tf, dir, sf, max, *templ);
  }
  else
  {
    std::cout << stats::CompileGroupRanks(section, tf, dir, sf, max, *templ);
  }
  
  return 0;
}
