//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <sstream>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/optional.hpp>
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

namespace util
{
template <> const char* util::EnumStrings<Who>::values[] = 
{
  "users",
  "groups",
  ""
};
}

void DisplayHelp(char* argv0, po::options_description& desc)
{
  std::cout << "usage: " << argv0 << " [options]" << std::endl;
  std::cout << desc;
}

void DisplayVersion()
{
  std::cout << "ebftpd ranks " + std::string(version) << std::endl;
}

namespace stats
{

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

std::ostream& operator<<(std::ostream& os, const stats::Timeframe& tf)
{
  return (os << util::EnumToString(tf));
}

std::ostream& operator<<(std::ostream& os, const stats::Direction& dir)
{
  return (os << util::EnumToString(dir));
}

std::ostream& operator<<(std::ostream& os, const stats::SortField& sf)
{
  return (os << util::EnumToString(sf));
}

} /* stats namespace */

std::istream& operator>>(std::istream& is, Who& who)
{
  std::string token;
  is >> token;
  if (!util::EnumFromString(token, who))
  {
    throw po::validation_error(po::validation_error::invalid_option_value);
  }
  return is;
}

std::ostream& operator<<(std::ostream& os, const Who& who)
{
  return (os << util::EnumToString(who));
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
    ("raw,r", "raw formatting")
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
  
  raw = vm.count("raw") > 0;
  
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
    std::cout << stats::CompileUserRanks(section, tf, dir, sf, max, *templ) << std::endl;
  }
  else
  {
    std::cout << stats::CompileGroupRanks(section, tf, dir, sf, max, *templ) << std::endl;
  }
  
  return 0;
}
