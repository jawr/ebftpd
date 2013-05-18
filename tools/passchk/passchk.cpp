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
#include "util/passwd.hpp"
#include "cfg/get.hpp"
#include "cfg/error.hpp"
#include "version.hpp"
#include "db/connection.hpp"

std::shared_ptr<cfg::Config> config;

void DisplayHelp(char* argv0, boost::program_options::options_description& desc)
{
  std::cout << "usage: " << argv0 << " [options] <username> <password>" << std::endl;
  std::cout << desc;
}

void DisplayVersion()
{
  std::cout << "ebftpd passchk " + std::string(version) << std::endl;
}

bool ParseOptions(int argc, char** argv, std::string& configPath, 
                  std::string& username, std::string& password,
                  bool& quiet)
{
  namespace po = boost::program_options;
  po::options_description visible("supported options");
  visible.add_options()
    ("help,h", "display this help message")
    ("version,v", "display version")
    ("quiet,q", "suppress output")
    ("config-path,c", po::value<std::string>(), "specify location of config file")
  ;
  
  po::options_description all("positional options");
  all.add(visible);
  all.add_options()
    ("username", po::value<std::string>(&username)->required(), "username")
    ("password", po::value<std::string>(&password)->required(), "password")
  ;

  po::positional_options_description pos;
  pos.add("username", 1);
  pos.add("password", 1);

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

    po::notify(vm);
  }
  catch (const boost::program_options::error& e)
  {
    std::cerr << e.what() << std::endl;
    DisplayHelp(argv[0], visible);
    return false;
  }

  quiet = vm.count("quiet") > 0;
  if (vm.count("config-path")) configPath = vm["config-path"].as<std::string>();
  
  return true;
}

bool RetrieveHashAndSalt(const std::string& username, std::string& hash, std::string& salt)
{
  try
  {
    db::SafeConnection conn;
    try
    {
      const auto& dbConfig = config->Database();
      auto fields = BSON("password" << 1 << "salt" << 1);
      auto query = QUERY("name" << username);
      auto cursor = conn.BaseConn().query(dbConfig.Name() + ".users", query, 1, 0, &fields);
      if (!cursor.get()) throw mongo::DBException("Connection failure", 0);
      if (!cursor->more())
      {
        std::cout << "User " << username << " doesn't exist." << std::endl;
        return false;
      }
      
      auto result = cursor->next();
      hash = result["password"].String();
      salt = result["salt"].String();
    }
    catch (const mongo::DBException& e)
    {
      std::cerr << "Unable to query database: " << e.what() << std::endl;
      return false;
    }
  }
  catch (const db::DBError& e)
  {
    std::cerr << e.what() << std::endl;
    return false;
  }
  
  return true;
}

int main(int argc, char** argv)
{
  std::string username;
  std::string password;
  std::string configPath;
  bool quiet;

  if (!ParseOptions(argc, argv, configPath, username, password, quiet)) return 1;

  try
  {
    config = cfg::Config::Load(configPath, true);
    cfg::UpdateShared(config);
  }
  catch (const cfg::ConfigError& e)
  {
    std::cerr << "Failed to load config: " << e.Message() << std::endl;
    return 1;
  }
  
  std::string hash;
  std::string salt;
  if (!RetrieveHashAndSalt(username, hash, salt)) return 1;
  
  using namespace util::passwd;
  if (HexEncode(HashPassword(password, HexDecode(salt))) != hash)
  {
    if (!quiet) std::cout << "Password incorrect" << std::endl;
    _exit(1);
  }
  
  if (!quiet) std::cout << "Password okay" << std::endl;
  _exit(0); // used to work around mongodb driver bug
}
