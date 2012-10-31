#include "util/time.hpp"

namespace util
{

Time Time::instance;

void Time::Update()
{
  boost::mutex::scoped_lock lock(instance.mtx);
  instance.now = boost::posix_time::second_clock::local_time();
}
// end
}
