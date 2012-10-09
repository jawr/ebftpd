#include "db/task.hpp"
#include "db/worker.hpp"
#include "logger/logger.hpp"
namespace db
{
void Select::Execute(Worker& worker)
{
   logger::ftpd << "SELECT EXECUTE" << logger::endl;

}
// end
}
