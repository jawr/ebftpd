#include "db/pool.hpp"
#include "db/exception.hpp"
#include "logger/logger.hpp"
#include "cfg/get.hpp"
#include <tr1/memory>
#include "db/task.hpp"
namespace db
{

Pool Pool::instance;

void Pool::Run()
{
  logger::ftpd << "db::Pool started" << logger::endl;

  for (int i = 0; i < 8; ++i)
  {
    try
    {
      std::unique_ptr<Worker> worker(new Worker("localhost", queue));
      worker->Start();
      workers.push_back(worker.release());
    }
    catch (const DBError& e)
    {
       throw e;
    }
  }

  while (true)
  {
    TaskQueue::size_type size = queue.WaitChanged();
  }
    
}

// end
}

#ifdef DB_POOL_TEST
#include <mongo/client/dbclient.h>
#include <boost/thread/future.hpp>
#include "db/interface.hpp"
#include "acl/types.hpp"
#include "acl/usercache.hpp"
#include "acl/groupcache.hpp"
#include <iostream>
int main()
{
  db::Pool::StartThread();

  db::Initalize();

  acl::UserCache::Initalize();
  acl::GroupCache::Initalize();

  acl::UserCache::Create("iotest", "password", "123");
  acl::GroupCache::Create("TESTGRP");
  acl::UserCache::Create("test2", "wowow", "");
  acl::UserCache::SetPrimaryGID("iotest", 
    acl::GroupCache::Group("TESTGRP").GID());
  
  
  db::Pool::JoinThread();
  
}
#endif 
