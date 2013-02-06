#ifndef __DB_USERDB_HPP
#define __DB_USERDB_HPP

#include <iostream>

namespace acl { namespace recode
{
class User;
} 
} 

namespace db { namespace recode
{

class UserDB
{
  acl::recode::User& user;

  bool Unserialize(const mongo::BSONObj& obj);
  mongo::Query DefaultQuery();
  
public:
  UserDB(acl::recode::User& user) : user(user) { }

  bool Load();
  void SaveName() { std::cout << "SaveName" << std::endl; }
  void AddIPMask(const std::string& ipMask) { std::cout << "AddIPMask" << std::endl; }
  void DelIPMask(const std::string& ipMask) { std::cout << "DelIPMask" << std::endl; }
};

} /* recode namespace */
} /* db namespace */

#endif
