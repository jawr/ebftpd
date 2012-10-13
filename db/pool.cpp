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
std::cout << "launch worker" << std::endl;
}

void Pool::KillWorker()
{
std::cout << "killworker" << std::endl;
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

  TaskQueue::size_type queueSize = 0;
  while (true)
  {
    queueSize = queue.WaitChanged(queueSize);

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

#include <iostream>
#include <sstream>
#include <mongo/client/dbclient.h>
#include <boost/thread/future.hpp>
#include "db/interface.hpp"
#include "acl/types.hpp"
#include "acl/usercache.hpp"
#include "acl/groupcache.hpp"
#include "stats/stat.hpp"

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

  db::IncrementStats(acl::UserCache::User("iotest"),
    21474836, 23.3, stats::Direction::Upload);
  db::IncrementStats(acl::UserCache::User("iotest"),
    836, 2.3, stats::Direction::Upload);
  db::IncrementStats(acl::UserCache::User("iotest"),
    474836, 213.3, stats::Direction::Download);

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
