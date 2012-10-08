#include <memory>
#include "db/pool.hpp"
#include "db/exception.hpp"
#include "logger/logger.hpp"
#include "cfg/get.hpp"
#include "db/task.hpp"

namespace db
{

Pool Pool::instance;

Pool::Pool() :
  maxWorkers(boost::thread::hardware_concurrency()),
  started(false)
{
  if (!maxWorkers) maxWorkers = defaultMaxWorkers;
}

void Pool::LaunchWorker()
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

void Pool::KillWorker()
{
  workers.back().Stop(true);
  workers.pop_back();
}

void Pool::Finalise()
{
  queue.WaitEmpty();
  while (!workers.empty()) KillWorker();
}

void Pool::HandleWorkers(size_t queueSize)
{
  if (!queueSize)
  {
    while (workers.size() > 1) KillWorker();
  }
  else if (queueSize > maxQueueContention &&
           workers.size() < maxWorkers)
  {
    LaunchWorker();
  }
}

void Pool::Run()
{
  LaunchWorker();

  TaskQueue::size_type queueSize;
  while (true)
  {
    queueSize = queue.WaitChanged();

    {
      boost::this_thread::disable_interruption noInterrupt;
      HandleWorkers(queueSize);
    }
  }    
}

void Pool::StartThread()
{
  instance.Start();
}

void Pool::StopThread()
{
  instance.Stop(true);
  instance.Finalise();
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
#include <sstream>

void ThreadMain()
{
  for (int i = 0; i < 20; ++i)
    db::GetNewUserID();
}

int main()
{
  db::Initalize();

  acl::UserCache::Initalize();
  acl::GroupCache::Initalize();

  acl::UserCache::Create("iotest", "password", "123");
  acl::GroupCache::Create("TESTGRP");
  acl::UserCache::Create("test2", "wowow", "");
  acl::UserCache::SetPrimaryGID("iotest", 
  acl::GroupCache::Group("TESTGRP").GID());

  boost::thread threads[20];

  for (int i = 0; i < 20; ++i)
  {
    threads[i] = boost::thread(&ThreadMain);
  }  
  
  for (int i = 0; i < 20; ++i)
    threads[i].join();
  
  db::Pool::StopThread();
  
}
#endif 
