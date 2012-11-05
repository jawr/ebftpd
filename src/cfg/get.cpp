#include <memory>
#include <cassert>
#include <boost/thread/tss.hpp>
#include <boost/thread/mutex.hpp>
#include "cfg/get.hpp"
#include "cfg/config.hpp"

namespace cfg
{

namespace
{

boost::thread_specific_ptr<Config> thisThread;

boost::mutex sharedMutex;
std::shared_ptr<Config> shared;

}

void UpdateShared(const std::shared_ptr<Config> newShared)
{
  boost::lock_guard<boost::mutex> lock(sharedMutex);
  shared = newShared;
}

void UpdateLocal()
{
  Config* config = thisThread.get();
  boost::lock_guard<boost::mutex> lock(sharedMutex);
  if (config && shared->Version() <= config->Version()) return;
  thisThread.reset(new Config(*shared));
}

const Config& Get()
{
  assert(shared.get()); // program must never call Get until a valid config is loaded
  Config* config = thisThread.get();
  if (!config)
  {
    UpdateLocal();
    config = thisThread.get();
    assert(config);
  }
  return *config;
}

}

#ifdef CFG_GET_TEST

#include <iostream>

int main()
{
  
}

#endif
