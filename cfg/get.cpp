#include <tr1/memory>
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
std::tr1::shared_ptr<Config> shared;

}

void UpdateShared(const std::tr1::shared_ptr<Config> newShared)
{
  boost::lock_guard<boost::mutex> lock(sharedMutex);
  shared = newShared;
}

const Config& Get(bool update)
{
  assert(shared.get()); // program must never call Get until a valid config is loaded
  Config* config = thisThread.get();
  if (config && !update) return *config;
  boost::lock_guard<boost::mutex> lock(sharedMutex);
  if (config && update && shared->Version() <= config->Version()) return *config;
  config = new Config(*shared);
  thisThread.reset(config);
  return *config;
}

}

#ifdef CFG_GET_TEST

#include <iostream>

int main()
{
  
}

#endif
