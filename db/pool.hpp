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
public:
  Pool() {};
  ~Pool() {};

  void Queue(TaskPtr task) { queue.Push(task); };

};

// end
}
#endif
