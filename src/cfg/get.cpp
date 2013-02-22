#include <memory>
#include <cassert>
#include <boost/thread/tss.hpp>
#include <mutex>
#include <boost/signals2.hpp>
#include "cfg/get.hpp"
#include "cfg/config.hpp"
#include "logs/logs.hpp"

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

bool RequireStopStart()
{
  bool required = false;
  const Config& old = cfg::Get();
  
  if (shared->ValidIp() != old.ValidIp())
  {
    logs::Error("'valid_ip' config option changed, full stop start required.");
    required = true;
  }
  
  if (shared->Port() != old.Port())
  {
    logs::Error("'port' config option changed, full stop start required.");
    required = true;
  }
  
  if (shared->TlsCertificate() != old.TlsCertificate())
  {
    logs::Error("'tls_certificate' option changed, full stop start required.");
    required = true;
  }

  if (shared->TlsCiphers() != old.TlsCiphers())
  {
    logs::Error("'tls_ciphers' option changed, full stop start required.");
    required = true;
  }

  if (shared->Database().Address() != old.Database().Address() ||   
      shared->Database().Port() != old.Database().Port())
  {
    logs::Error("'database' option changed, full stop start required.");
    required = true;
  }
      
  return required;
}

void ConnectUpdatedSlot(const std::function<void()>& slot)
{
  updated.connect(slot);
}

}
