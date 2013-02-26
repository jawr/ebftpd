#include <iostream>
#include <sstream>
#include <mongo/client/dbclient.h>
#include "util/passwd.hpp"
#include "cfg/config.hpp"
#include "cfg/error.hpp"

std::shared_ptr<cfg::Config> config;

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
  if (argc != 4)
  {
    std::cout << "usage: " << argv[0] << " <username> <password> <config path>" << std::endl;
    return 1;
  }
  
  std::string username(argv[1]);
  std::string password(argv[2]);
  std::string configPath(argv[3]);
  
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
    std::cout << "NOMATCH" << std::endl;
    return 1;
  }
  
  std::cout << "MATCH" << std::endl;
  return 0;
}
