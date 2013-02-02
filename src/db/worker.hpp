#ifndef __DB_WORKER_HPP
#define __DB_WORKER_HPP

#include <mongo/client/dbclient.h>
#include "util/thread.hpp"

namespace db
{

class TaskQueue;

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
