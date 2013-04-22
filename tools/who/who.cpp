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
#include "text/error.hpp"
#include "text/parser.hpp"
#include "cmd/online.hpp"
#include "ftp/online.hpp"
#include "util/enumstrings.hpp"

namespace po = boost::program_options;

enum class Mode
{
  Normal,
  Siteop,
  Totals
};

namespace util
{
template <> const char* util::EnumStrings<Mode>::values[] = 
{
  "normal",
  "siteop",
  "totals",
  ""
};
}

std::istream& operator>>(std::istream& is, Mode& m)
{
  std::string token;
  is >> token;
  if (!util::EnumFromString(token, m))
  {
    throw po::validation_error(po::validation_error::invalid_option_value);
  }
  return is;
}

std::ostream& operator<<(std::ostream& os, const Mode& m)
{
  return (os << util::EnumToString(m));
}

void DisplayHelp(char* argv0, po::options_description& desc)
{
  std::cout << "usage: " << argv0 << " [options] [<user>]" << std::endl;
  std::cout << desc;
}

void DisplayVersion()
{
  std::cout << "ebftpd ranks " + std::string(version) << std::endl;
}

bool ParseOptions(int argc, char** argv, std::string& configPath, 
                  Mode& mode, bool& raw, std::string& templatePath,
                  std::string& user)
{
  po::options_description visible("supported options");
  visible.add_options()
    ("help,h", "display this help message")
    ("version,v", "display version")
    ("config-path,c", po::value<std::string>(&configPath), "specify location of config file")
    ("raw,r", "raw formatting")
    ("mode,m", po::value<Mode>(&mode)->default_value(Mode::Normal, "normal"), "normal, siteop, totals")
    ("template,y", po::value<std::string>(&templatePath), "template file path")
  ;
  
  std::string who;
  po::options_description all("positional options");
  all.add(visible);
  all.add_options()
    ("user", po::value<std::string>(&user), "username")
  ;

  po::positional_options_description pos;
  pos.add("user", 1);

  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(argc, argv).options(all).positional(pos).run(), vm);

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

std::string TemplateFilename(Mode mode, bool raw)
{
  std::string filename;
  if (raw) filename += "raw";
  
  switch (mode)
  {
    case Mode::Normal : filename += "who"; break;
    case Mode::Siteop : filename += "swho"; break;
    case Mode::Totals : filename += "twho"; break;
  }
  
  filename += ".tmpl";
  return filename;
}

int main(int argc, char** argv)
{
  Mode mode;
  bool raw;
  std::string configPath;
  std::string templatePath;
  std::string user;
  
  if (!ParseOptions(argc, argv, configPath, mode, raw, templatePath, user))
  {
    return 1;
  }

  try
  {
    cfg::UpdateShared(cfg::Config::Load(configPath, true));
  }
  catch (const cfg::ConfigError& e)
  {
    std::cerr << "Failed to load config: " << e.Message() << std::endl;
    return 1;
  }
  
  if (cfg::Get().Pidfile().empty())
  {
    std::cerr << "There must be a pid file set in your config to use this tool." << std::endl;
    return 1;
  }
  
  std::string id;
  if (!ftp::SharedMemoryID(cfg::Get().Pidfile(), id))
  {
    std::cerr << "Unable to load pid from pid file: " << cfg::Get().Pidfile() << std::endl;
    return 1;
  }
  
  if (templatePath.empty())
  {
    templatePath = cfg::Get().Datapath() + "/text/" + TemplateFilename(mode, raw);
  }
  
  boost::optional<text::Template> templ;
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

  std::cout << cmd::CompileWhosOnline(id, *templ, user) << "\n";
  
  return 0;
}
