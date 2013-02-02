#include <memory>
#include <cassert>
#include <boost/thread/tss.hpp>
#include <boost/thread/mutex.hpp>
#include "cfg/get.hpp"
#include "cfg/config.hpp"
#include "logs/logs.hpp"

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

bool RequireStopStart()
{
  bool required = false;
  const Config& old = cfg::Get();
  
  if (shared->ValidIp() != old.ValidIp())
  {
    logs::error << "'valid_ip' config option changed, full stop start required." << logs::endl;
    required = true;
  }
  
  if (shared->Port() != old.Port())
  {
    logs::error << "'port' config option changed, full stop start required." << logs::endl;
    required = true;
  }
  
  if (shared->TlsCertificate() != old.TlsCertificate())
  {
    logs::error << "'tls_certificate' option changed, full stop start required." << logs::endl;
    required = true;
  }
  
  if (shared->TlsCiphers() != old.TlsCiphers())
  {
    logs::error << "'tls_ciphers' option changed, full stop start required." << logs::endl;
    required = true;
  }

  if (shared->Database().Address() != old.Database().Address() ||   
      shared->Database().Port() != old.Database().Port())
  {
    logs::error << "'database' option changed, full stop start required." << logs::endl;
    required = true;
  }
      
  return required;
}

}
