#include <cassert>
#include "ftp/transfercounter.hpp"
#include "cfg/get.hpp"
#include "ftp/counter.hpp"

namespace ftp
{

CounterResult TransferCounter::Start(acl::UserID uid, int limit, bool exempt)
{
  int maxGlobal = getMaxGlobal();
  
  std::lock_guard<std::mutex> lock(mutex);
  int& count = personal[uid];
  if (count >= limit && limit != -1) return CounterResult::PersonalFail;
  if (!exempt && maxGlobal != -1 && global >= maxGlobal)
  {
    return CounterResult::GlobalFail;
  }
  ++count;
  ++global;
  return CounterResult::Okay;
}

void TransferCounter::Stop(acl::UserID uid)
{
  std::lock_guard<std::mutex> lock(mutex);
  int& count = personal[uid];
  assert(count > 0);
  --count;
  assert(global > 0);
  --global;
}

} /* ftp namespace */

