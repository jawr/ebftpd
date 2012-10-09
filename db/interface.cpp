#include "db/interface.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "db/types.hpp"
#include "acl/types.hpp"
namespace db
{
acl::User GetNewUserID()
{
  TaskPtr task(new db::Select("users"
  
  
}

void SaveUser(const acl::User& user)
{

}

// end
}
