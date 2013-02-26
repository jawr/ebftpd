#include <iostream>
#include <sstream>
#include <mongo/client/dbclient.h>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include "util/passwd.hpp"
#include "cfg/config.hpp"
#include "cfg/error.hpp"
#include "version.hpp"

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
                  std::string& username, std::string& password)
{
  namespace po = boost::program_options;
  po::options_description visible("supported options");
  visible.add_options()
    ("help,h", "display this help message")
    ("version,v", "display version")
    ("config-path,c", po::value<std::string>(), "specify location of config file")
  ;
  
  std::string who;
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

  if (vm.count("config-path")) configPath = vm["config-path"].as<std::string>();
  
  return true;
}

bool RetrieveHashAndSalt(const std::string& username, std::string& hash, std::string& salt)
{
  const auto& dbConfig = config->Database();
  mongo::DBClientConnection conn;
  try
  {
    conn.connect(dbConfig.Host());
    if (!dbConfig.Login().empty())
    {
      std::string errmsg;
      if (!conn.auth(dbConfig.Name(), dbConfig.Login(), dbConfig.Password(), errmsg))
        throw mongo::DBException("Authentication failed", 0);
    }

    auto fields = BSON("password" << 1 << "salt" << 1);
    auto query = QUERY("name" << username);
    auto cursor = conn.query(dbConfig.Name() + ".users", query, 1, 0, &fields);
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
    std::cout << "Unable to query database: " << e.what() << std::endl;
    return false;
  }
  
  return true;
}

int main(int argc, char** argv)
{
  std::string username;
  std::string password;
  std::string configPath;

  if (!ParseOptions(argc, argv, configPath, username, password)) return 1;

  try
  {
    config = cfg::Config::Load(configPath, true);
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
    std::cout << "Password incorrect" << std::endl;
    return 1;
  }
  
  std::cout << "Password okay" << std::endl;
  return 0;
}
