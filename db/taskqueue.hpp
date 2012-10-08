#ifndef __DB_TASKQUEUE_HPP
#define __DB_TASKQUEUE_HPP

#include "db/types.hpp"
#include <memory>
#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

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
    }
    
    changed.notify_one();
    return task;
  }
  
  void Push(const TaskPtr& task)
  {
    {
      boost::lock_guard<boost::mutex> lock(mutex);
      queue.push(task);
    }
    
    newTask.notify_all();
    changed.notify_one();
  }
  
  size_type WaitChanged()
  {
    boost::unique_lock<boost::mutex> lock(mutex);
    changed.wait(lock);
    return queue.size();
  }
};

} /* db namespace */

#endif
