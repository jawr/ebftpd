#include <cassert>
#include <boost/thread/locks.hpp>
#include "ftp/logincounter.hpp"
#include "cfg/get.hpp"
#include "ftp/counter.hpp"

namespace ftp
{

CounterResult LoginCounter::Start(acl::UserID uid, int limit, bool kickLogin, bool exempt)
{
  const cfg::Config& config = cfg::Get();
  boost::lock_guard<boost::mutex> lock(mutex);
  int& count = personal[uid];
  if (limit != -1 && count - kickLogin >= limit)
  {
    return CounterResult::PersonalFail;
  }
  int maxUsers = config.MaxUsers().Users();
  if (exempt) maxUsers += config.MaxUsers().ExemptUsers();
  if (global - kickLogin > maxUsers)
  {
    return CounterResult::GlobalFail;
  }
  ++count;
  ++global;
  return CounterResult::Okay;
}

void LoginCounter::Stop(acl::UserID uid)
{
  boost::lock_guard<boost::mutex> lock(mutex);
  int& count = personal[uid];
  assert(count > 0);
  --count;
  assert(global > 0);
  --global;
}

} /* ftp namespace */

