#ifndef __DB_TASKQUEUE_HPP
#define __DB_TASKQUEUE_HPP

#include <memory>
#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include "db/types.hpp"
#include "logs/logs.hpp"

namespace db
{

class TaskQueue
{
  std::queue<TaskPtr> queue;
  boost::mutex mutex;
  boost::condition_variable newTask;
  boost::condition_variable changed;
  
public:
  typedef std::queue<TaskPtr>::size_type size_type;

  TaskQueue() {};
  
  TaskPtr Pop()
  {
    TaskPtr task;
    
    {
      boost::unique_lock<boost::mutex> lock(mutex);
      while (queue.empty()) newTask.wait(lock);
      task = queue.front();
      queue.pop();
      logs::debug << "database task popped: " << queue.size() << logs::endl;
    }
    
    changed.notify_one();
    return task;
  }
  
  void Push(const TaskPtr& task)
  {
    logs::debug << "new database task pushed." << logs::endl;

    {
      boost::lock_guard<boost::mutex> lock(mutex);
      queue.push(task);
    }
    
    newTask.notify_all();
    changed.notify_one();
  }
  
  size_type WaitChanged(size_type lastSize)
  {
    boost::unique_lock<boost::mutex> lock(mutex);
    if (queue.size() == lastSize) changed.wait(lock);
    return queue.size();
  }
  
  void WaitEmpty()
  {
    boost::unique_lock<boost::mutex> lock(mutex);
    while (!queue.empty()) changed.wait(lock);
  }
};

} /* db namespace */

#endif
