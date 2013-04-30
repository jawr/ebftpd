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

#include <string>
#include <iostream>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/optional.hpp>
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "cfg/error.hpp"
#include "util/path/status.hpp"
#include "util/string.hpp"
#include "db/nuking/nuking.hpp"
#include "cmd/site/nuking.hpp"
#include "acl/user.hpp"
#include "fs/path.hpp"
#include "fs/directory.hpp"
#include "text/error.hpp"
#include "text/parser.hpp"
#include "logs/logs.hpp"
#include "version.hpp"

std::shared_ptr<cfg::Config> config;

void DisplayHelp(char* argv0, boost::program_options::options_description& desc)
{
  std::cout << "usage: " << argv0 << " [options] <path> <message> [<multiplier>|<percent>%]" << std::endl;
  std::cout << desc;
}

void DisplayVersion()
{
  std::cout << "ebftpd nuke " + std::string(version) << std::endl;
}

bool ParseOptions(int argc, char** argv, std::string& configPath,
                  std::string& templatePath, std::string& path, 
                  std::string& reason, int& multiplier, bool& isPercent,
                  std::string& nuker, bool& isUnnuke, bool& quiet)
{
  namespace po = boost::program_options;
  
  std::string multiStr;
  po::options_description visible("supported options");
  visible.add_options()
    ("help,h", "display this help message")
    ("version,v", "display version")
    ("config-path,c", po::value<std::string>(&configPath), "specify location of config file")
    ("unnuke,u", "do unnuke")
    ("nuker,n", po::value<std::string>(&nuker), "specify nuker")
    ("quiet,q", "suppress output")
    ("template,y", po::value<std::string>(&templatePath), "template file path")
  ;

  po::options_description all("positional options");
  all.add(visible);
  all.add_options()
    ("path", po::value<std::string>(&path)->required(), "path")
    ("reason", po::value<std::string>(&reason)->required(), "reason")
    ("multiplier", "multiplier")
  ;

  po::positional_options_description pos;
  pos.add("path", 1);
  pos.add("reason", 1);
  pos.add("multiplier", 1);

  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(argc, argv).options(all).
              positional(pos).run(), vm);

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
    
    if (vm.count("unnuke"))
    {
      if (vm.count("multiplier") > 0)
      {
        throw boost::program_options::error("multiplier cannot be specified for unnukes");
      }
      
      if (vm.count("percent") > 0)
      {
        throw boost::program_options::error("percent option cannot be used for unnukes");
      }
    }
    else
    if (vm.count("multiplier") == 0)
    {
      throw boost::program_options::error("multiplier must be specified for nukes");
    }
    
    if (vm.count("multiplier") > 0)
    {
      auto multiStr = vm["multiplier"].as<std::string>();
      
      bool multiOkay = true;
      size_t pos;
      try
      {
        multiplier = std::stoi(multiStr, &pos);
        if (multiplier < 0)
          multiOkay = false;
        else
        if (pos < multiStr.length())
        {
          if (multiStr.substr(pos) == "%")
            isPercent = true;
          else
           multiOkay = false;
        }
      }
      catch (const std::logic_error&)
      {
        multiOkay = false;
      }
      
      if (!multiOkay)
      {
        throw boost::program_options::error("invalid multiplier");
      }
    }
    
    po::notify(vm);
  }
  catch (const boost::program_options::error& e)
  {
    std::cerr << e.what() << std::endl;
    DisplayHelp(argv[0], visible);
    return false;
  }

  isPercent = vm.count("percent") > 0;
  isUnnuke = vm.count("unnuke") > 0;
  quiet = vm.count("quiet") > 0;
  return true;
}

fs::VirtualPath VirtualisePath(const std::string& path)
{
  assert(!path.empty());
  char realWorkDir[PATH_MAX];
  char * p = getcwd(realWorkDir, sizeof(realWorkDir)); (void) p;
  return fs::MakeVirtual(fs::RealPath(util::path::Resolve(util::path::Join(realWorkDir, path))));
}

int main(int argc, char** argv)
{
  std::string path;
  std::string reason;
  int multiplier;
  bool isPercent;
  std::string configPath;
  std::string templatePath;
  std::string nuker;
  bool isUnnuke;
  bool quiet;
  
  if (!ParseOptions(argc, argv, configPath, templatePath, path, reason, 
                    multiplier, isPercent, nuker, isUnnuke, quiet)) return 1;
  
  if (quiet)
  {
    std::cout.setstate(std::ios::failbit);
    std::cerr.setstate(std::ios::failbit);
  }
  
  std::clog.setstate(std::ios::failbit);
  
  logs::InitialisePreConfig();
  
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
  logs::InitialisePostConfig();

  acl::UserID nukerUID = 0;
  if (!nuker.empty())
  {
    nukerUID = acl::NameToUID(nuker);
    if (nukerUID < 0)
    {
      std::cerr << "Nuker " << nuker << " doesn't exist." << std::cerr;
      return 1;
    }
  }
  
  boost::optional<text::Template> templ;
  try
  {
    if (templatePath.empty())
    {
      templatePath = config->Datapath() + "/text/";
      if (isUnnuke) templatePath += "unnuke";
      else templatePath += "nuke";
      templatePath += ".tmpl";
    }
    templ.reset(text::TemplateParser(templatePath).Create());
  }
  catch (const text::TemplateError& e)
  {
    std::cerr << "Unable to load template file: " << e.what() << std::endl;
    return 1;
  }
  
  try
  {
    auto vpath = VirtualisePath(path);
    
    try
    {
      using namespace cmd::site;
      
      if (!isUnnuke)
      {
        std::cout << NukeTemplateCompile(Nuke(vpath, multiplier, isPercent, reason, nukerUID),
                                         *templ, vpath) << std::endl;
      }
      else
      {
        std::cout << NukeTemplateCompile(Unnuke(vpath, reason, nukerUID), *templ, vpath) << std::endl;
      }
    }
    catch (const util::RuntimeError& e)
    {
      std::cerr << "Error while " << (isUnnuke ? "un" : "" ) 
                << "nuking: " << path << ": " << e.what() << std::endl;
      return 1;
    }
    
  }
  catch (const std::logic_error& e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  
  return 0;
}
