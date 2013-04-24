//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <cstring>
#include <sstream>
#include <functional>
#include "plugin/plugin.hpp"
#include "logs/logs.hpp"
#include "util/path/path.hpp"
#include "plugin/error.hpp"
#include "cfg/get.hpp"
#include "util/verify.hpp"
#include "plugin/hooks.hpp"
#include "main.hpp"

namespace plugin
{

boost::once_flag PluginManager::instanceOnce;
std::unique_ptr<PluginManager> PluginManager::instance;
__thread Plugin* PluginState::currentPlugin;

Plugin::Plugin(const PluginDriver& driver) : 
  driver(driver),
  commandHooks(new ::plugin::CommandHooks()),
  eventHooks(new ::plugin::EventHooks())
{
}

Plugin::~Plugin()
{
}

const char* Plugin::Name() const
{
  return driver->Name();
}

void Plugin::LoadScripts()
{
  std::string name = util::ToLowerCopy(Name());
  const cfg::Config& config = cfg::Get();
  auto it = std::find_if(config.Plugins().begin(), config.Plugins().end(),
              [&name](const cfg::Plugin& pluginConfig)
              {
                return pluginConfig.Name() == name;
              });

  verify(it != config.Plugins().end());
  const cfg::Plugin& pluginConfig = *it;
  
  ScopeSwapPlugin swapGuard(*this); (void) swapGuard;
  for (const std::string& script : pluginConfig.Scripts())
  {
    try
    {
      LoadScript(util::path::Join(config.Scriptpath(), script));
    }
    catch (const PluginError& e)
    {
      logs::Error("Failed to load script: %1%: %2%", name, e.what());
    }
  }
}

PluginDriver::PluginDriver(const std::string& path) :
  data(new Data())
{
  data->handle = dlopen(path.c_str(), RTLD_NOW);
  if (!data->handle) throw InitialiseError(dlerror());
  
  void* function = dlsym(data->handle, "CreateFactory");
  if (!function) throw InitialiseError(dlerror());
  
  PluginFactory* (*create)();
  *reinterpret_cast<void**>(&create) = function;
  data->factory = create();
  
  if (strcmp(data->factory->ServerVersion(), version))
  {
    std::ostringstream os;
    os << "Plugin (" << data->factory->ServerVersion() 
        << ") <-> server (" << version << ") version mismatch";
    throw InitialiseError(os.str());
  }
}

PluginDriver::Data::~Data()
{
  delete factory;
  if (handle) dlclose(handle);
}

PluginManager::~PluginManager()
{
  verify(drivers.empty());
}

boost::optional<std::pair<CommandHook, Plugin*>> 
PluginCollection::LookupCommand(const std::string& command)
{
  boost::optional<std::pair<CommandHook, Plugin*>> result;
  for (auto plugin : data->plugins)
  {
    auto hook = plugin->CommandHooks().Lookup(command);
    if (hook)
    {
      result.reset(std::make_pair(*hook, plugin));
      break;
    }
  }
  return result;
}

bool PluginCollection::TriggerEvent(Event event, ftp::Client& client, const EventHookArgs& args)
{
  Client pluginClient(client);
  bool okay = true;
  for (auto plugin : data->plugins)
  {
    ScopeSwapPlugin swapGuard(*plugin); (void) swapGuard;
    plugin->EventHooks().Trigger(event, pluginClient, args, okay);
  }
  return okay;
}

bool PluginCollection::TriggerEvent(Event event, ftp::Client& client)
{
  return TriggerEvent(event, client, EventHookArgs());
}


PluginCollection PluginManager::CreatePlugins() const
{
  PluginCollection collection;
  
  {
    boost::shared_lock<boost::shared_mutex> lock(mutex);
    for (auto& driver : drivers)
    {
      collection.data->plugins.emplace_back(driver->Create(driver));
    }
  }
  
  return collection;
}

void PluginManager::Reload()
{
  const cfg::Config& config = cfg::Get();
  
  boost::unique_lock<boost::shared_mutex> lock(mutex);
  UnloadAllNoLock();
  
  for (auto& pluginConfig : config.Plugins())
  {
    try
    {
      drivers.emplace_back(util::path::Join(config.Pluginpath(), pluginConfig.Name()) + ".so");
      logs::Debug("Loaded plugin: %1% %2%", drivers.back()->Name(), drivers.back()->PluginVersion());
    }
    catch (const PluginError& e)
    {
      logs::Error("Failed to load plugin: %1%: %2%", pluginConfig.Name(), e.what());
    }
  }
}

void PluginManager::UnloadAllNoLock()
{
  while (!drivers.empty())
  {
    auto driver = drivers.front();
    drivers.erase(drivers.begin());
    logs::Debug("Unloaded plugin: %1% %2%", driver->Name(), driver->PluginVersion());
  }
}

void PluginManager::UnloadAll()
{
  boost::unique_lock<boost::shared_mutex> lock(mutex);
  UnloadAllNoLock();
}

void Initialise()
{
  cfg::ConnectUpdatedSlot([]() { PluginManager::Get().Reload(); });
}

} /* plugin namespace */
