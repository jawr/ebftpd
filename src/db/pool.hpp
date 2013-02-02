#ifndef __DB_POOL_HPP
#define __DB_POOL_HPP

#include <memory>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/logic/tribool.hpp>
#include "db/worker.hpp"
#include "db/taskqueue.hpp"
#include "util/thread.hpp"

namespace db
{

class Task;
typedef std::shared_ptr<Task> TaskPtr;

class Pool : public util::Thread
{
  TaskQueue queue;
  boost::ptr_vector<Worker> workers;
  unsigned maxWorkers;
  
  boost::mutex startedMutex;
  boost::tribool started;
  
  virtual void Run();

  std::string host;

  static Pool instance;
  static const unsigned defaultMaxWorkers = 4;
  static const unsigned maxQueueContention = 2;
  
  void LaunchWorker();
  void KillWorker();
  void HandleWorkers(size_t queueSize);
  void Finalise();

public:
  Pool();

  static void Queue(const TaskPtr& task) { instance.queue.Push(task); }
  static void StartThread();
  static void StopThread();

};

// end
}

#endif
