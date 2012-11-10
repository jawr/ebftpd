#ifndef __DB_WORKER_HPP
#define __DB_WORKER_HPP

#include <mongo/client/dbclient.h>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include "util/thread.hpp"
#include "db/types.hpp"
#include "db/taskqueue.hpp"

namespace db
{

class Worker : public util::Thread
{
  mongo::DBClientConnection conn;
  TaskQueue& queue;

  virtual void Run();

public:
  Worker(const std::string& host, 
         const std::string& database,
         TaskQueue& queue, 
         const std::string& login, 
         const std::string& password);
};

}

#endif
