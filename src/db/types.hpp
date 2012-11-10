#ifndef __DB_TYPES_HPP
#define __DB_TYPES_HPP

#include <memory>
#include <vector>
#include <mongo/client/dbclient.h>

namespace db
{

class Task;
typedef std::shared_ptr<Task> TaskPtr;
typedef std::vector<mongo::BSONObj> QueryResults;

}

#endif

