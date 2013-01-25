#include <ctime>
#include <mongo/client/dbclient.h>
#include <boost/thread/future.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "db/interface.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "db/types.hpp"
#include "db/bson/user.hpp"
#include "db/bson/group.hpp"
#include "acl/types.hpp"
#include "acl/types.hpp"
#include "logs/logs.hpp"
#include "db/error.hpp"
#include "util/verify.hpp"

namespace db
{

void Initialize()
{
  db::Pool::StartThread();

  Pool::Queue(std::make_shared<db::EnsureIndex>("users", BSON("uid" << 1)));
  Pool::Queue(std::make_shared<db::EnsureIndex>("users", BSON("name" << 1)));
  Pool::Queue(std::make_shared<db::EnsureIndex>("groups", BSON("gid" << 1)));
  Pool::Queue(std::make_shared<db::EnsureIndex>("groups", BSON("name" << 1)));
  
  Pool::Queue(std::make_shared<db::EnsureIndex>("transfers", 
          BSON("uid" << 1 << "direction" << 1 << 
               "section" << 1 << "day" << 1 << 
               "week" << 1 << "month" << 1 << 
               "year" << 1)));
               
  Pool::Queue(std::make_shared<db::EnsureIndex>("ipmasks", BSON("uid" << 1)));
    
  Pool::Queue(std::make_shared<db::Insert>("globals", 
          BSON("_id" << "ipmasks modified" << "value" << mongo::Date_t()), true));
}

void Cleanup()
{
  Pool::StopThread();
}

// end
}
