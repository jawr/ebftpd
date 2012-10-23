#ifndef __FTP_TASK_TYPES_HPP
#define __FTP_TASK_TYPES_HPP

#include <memory>

namespace ftp { namespace task
{
class Task;
}

typedef std::shared_ptr<task::Task> TaskPtr;

// end
}
#endif
