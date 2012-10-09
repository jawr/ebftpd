#ifndef __DB_POOL_HPP
#define __DB_POOL_HPP
#include "db/task.hpp"
#include "db/worker.hpp"
#include "db/taskqueue.hpp"
#include "util/thread.hpp"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_deque.hpp>
#include <tr1/memory>
#include <memory>
namespace db
{

class Pool : public util::ThreadConsumer
{
  TaskQueue queue;

  boost::ptr_vector<Worker> workers;
  
  virtual void Run();

  std::string host;

  static Pool instance;

public:
  ~Pool() {};

  static void Queue(TaskPtr task) { instance.queue.Push(task); };
  static void StartThread() { instance.Start(); };
  static void JoinThread() { instance.Join(); };

};

// end
}
#endif
