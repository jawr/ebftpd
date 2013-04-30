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
#include "cfg/config.hpp"
#include "cfg/error.hpp"
#include "util/path/status.hpp"
#include "util/string.hpp"
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
                  std::string& path, std::string& reason, 
                  int& multiplier, bool& isPercent,
                  std::string& nuker)
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
  return true;
}

int main(int argc, char** argv)
{
  std::string path;
  std::string reason;
  int multiplier;
  bool isPercent;
  std::string configPath;
  std::string nuker;
  
  if (!ParseOptions(argc, argv, configPath, path, reason, multiplier, isPercent, nuker)) return 1;
}
