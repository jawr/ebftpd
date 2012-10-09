#include <tr1/memory>
#include <cassert>
#include <boost/thread/tss.hpp>
#include <boost/thread/mutex.hpp>
#include "db/get.hpp"
#include "db/pool.hpp"
namespace db
{
namespace
{
  boost::mutex sharedMutex;
  std::tr1::shared_ptr<Pool> shared;
}

void UpdateShared(const std::tr1::shared_ptr<Pool> newShared)
{
  boost::lock_guard<boost::mutex> lock(sharedMutex);
  shared = newShared;
}

Pool& Get()
{
  assert(shared.get()); // program should never call Get without being initalized
  Pool* pool = shared.get();
  return *pool;
}

// end
}
