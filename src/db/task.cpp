#include "db/task.hpp"
#include "db/worker.hpp"
#include "db/exception.hpp"
#include "logs/logs.hpp"
#include <boost/thread/future.hpp>
namespace db
{

void Update::Execute(Worker& worker)
{
  try
  {
    worker.Update(container, obj, query, upsert);
  }
  catch (const DBError& e)
  {
    logs::db << "Update failure: " << e.Message() << logs::endl;
  }
}

void Delete::Execute(Worker& worker)
{
  try
  {
    worker.Delete(container, query);
  }
  catch (const DBError& e)
  {
    logs::db << "Delete failure: " << e.Message() << logs::endl;
  }
}

void Select::Execute(Worker& worker)
{
  try
  {
    worker.Get(container, query, results, limit);
    promise.set_value(true);
  }
  catch (const DBError& e)
  {
    logs::db << "Select failure: " << e.Message() << logs::endl;
  }
}

void Insert::Execute(Worker& worker)
{
  try
  {
    worker.Insert(container, obj);
  }
  catch (const DBError& e)
  {
    logs::db << "Insert failure: " << e.Message() << logs::endl;
  }
}

void EnsureIndex::Execute(Worker& worker)
{
  try
  {
    worker.EnsureIndex(container, obj);
  }
  catch(const DBError& e)
  {
    logs::db << "Ensure index failure: " << e.Message() << logs::endl;
  }
}


// end
}
