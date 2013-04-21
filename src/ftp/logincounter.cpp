#include <cassert>
#include "ftp/logincounter.hpp"
#include "cfg/get.hpp"
#include "ftp/counter.hpp"

namespace ftp
{

CounterResult LoginCounter::Start(acl::UserID uid, int limit, bool kickLogin, bool exempt)
{
  std::lock_guard<std::mutex> lock(mutex);
  int& count = personal[uid];
  if (limit != -1 && count - kickLogin >= limit)
  {
    return CounterResult::PersonalFail;
  }
  int maxUsers = cfg::Config::MaxOnline().Users();
  if (exempt) maxUsers += cfg::Config::MaxOnline().ExemptUsers();
  if (global - kickLogin > maxUsers)
  {
//    return CounterResult::GlobalFail;
  }
  ++count;
  ++global;
  return CounterResult::Okay;
}

bool LoginCounter::LoginsUsed(acl::UserID uid, int limit)
{
  std::lock_guard<std::mutex> lock(mutex);
  return limit != -1 && personal[uid] >= limit;
}

void LoginCounter::Stop(acl::UserID uid)
{
  std::lock_guard<std::mutex> lock(mutex);
  int& count = personal[uid];
  assert(count > 0);
  --count;
  assert(global > 0);
  --global;
}

int LoginCounter::GlobalCount() const
{
  std::lock_guard<std::mutex> lock(mutex);
  return global;
}

} /* ftp namespace */

