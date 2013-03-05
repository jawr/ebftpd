#include "plugin/locks.hpp"

namespace plugin
{

boost::thread_specific_ptr<Locker> LockManager::instance;

} /* plugin namespace */