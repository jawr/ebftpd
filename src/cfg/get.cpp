#include <cassert>
#include <boost/thread/tss.hpp>
#include <mutex>
#include <boost/signals2.hpp>
#include "cfg/get.hpp"
#include "logs/logs.hpp"
#include "util/string.hpp"
#include "cfg/error.hpp"

namespace cfg
{

namespace
{

boost::thread_specific_ptr<Config> thisThread;
std::mutex sharedMutex;
std::shared_ptr<Config> shared;
boost::signals2::signal<void()> updated;

}

void UpdateShared(const std::shared_ptr<Config> newShared)
{
  {
    std::lock_guard<std::mutex> lock(sharedMutex);
    shared = newShared;
  }
  
  updated();
}

void UpdateLocal()
{
  Config* config = thisThread.get();
  std::lock_guard<std::mutex> lock(sharedMutex);
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

void StopStartCheck()
{
  const Config& old = cfg::Get();
  std::vector<std::string> settings;

  if (shared->ValidIp() != old.ValidIp()) settings.push_back("valid_ip");
  if (shared->Port() != old.Port()) settings.push_back("port");
  if (shared->TlsCertificate() != old.TlsCertificate()) settings.push_back("tls_certificate");
  if (shared->TlsCiphers() != old.TlsCiphers()) settings.push_back("tls_ciphers");
  if (shared->Database().Address() != old.Database().Address() ||   
      shared->Database().Port() != old.Database().Port())
  {
    settings.push_back("database");
  }

  if (!settings.empty())
  {
    throw StopStartNeeded("Full stop start required for these settings: " + 
                          util::Join(settings, ","));
  }
}

void ConnectUpdatedSlot(const std::function<void()>& slot)
{
  updated.connect(slot);
}

}
