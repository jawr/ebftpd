#include <functional>
#include <dlfcn.h>
#include "plugin/plugin.hpp"
#include "plugin/error.hpp"
#include "util/scopeguard.hpp"
#include "cfg/get.hpp"
#include "util/path/path.hpp"
#include "logs/logs.hpp"

namespace plugin
{

std::unique_ptr<FactoryManager> FactoryManager::instance;

void FactoryManager::Rehash()
{
  const cfg::Config& config = cfg::Get();  
  if (config.Plugins().empty()) return;
  
  boost::lock_guard<boost::mutex> lock(mutex);
  Cleanup();
  
  for (const auto& plugin : config.Plugins())
  {
    try
    {
      auto holder = CreateFactory(util::path::Join(config.Pluginpath(), plugin.Name() + ".so"));
      factories.insert(std::make_pair(plugin.Name(), holder));
      logs::Debug("Loaded plugin: %1% %2%", holder.Factory().Name(), holder.Factory().Version());
    }
    catch (const InitialiseError& e)
    {
      logs::Error("Failed to load plugin: %1%: %2%", plugin.Name(), e.what());
    }
  }
}

void FactoryManager::Cleanup()
{
  while (!factories.empty())
  {
    Factory& factory = factories.begin()->second.Factory();
    logs::Error("Unloaded plugin: %1% %2%", factory.Name(), factory.Version());
    factories.erase(factories.begin());
  }
}

FactoryHolder CreateFactory(const std::string& path)
{
  void* library = dlopen(path.c_str(), RTLD_NOW);
  if (!library) throw InitialiseError(dlerror());
  
  std::shared_ptr<void> sharedLibrary(library, dlclose);
  void* function = dlsym(library, "CreateFactory");
  const char* error = dlerror();
  if (error) throw InitialiseError("Does not appear to be a valid plugin");
  
  Factory* (*create)();
  *reinterpret_cast<void**>(&create) = function;
  return FactoryHolder(path, sharedLibrary, std::shared_ptr<Factory>(create()));
}

void InitialiseFactories()
{
  cfg::ConnectUpdatedSlot([]() { FactoryManager::Get().Rehash(); });
}

} /* plugin namespace */
