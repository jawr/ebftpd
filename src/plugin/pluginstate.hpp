#include "plugin/plugin.hpp"
#include "cmd/site/factory.hpp"

namespace plugin
{

class Factory { };
class CommandHooks { };
class EventHooks { };

class Plugin
{
  std::string configName;
  std::shared_ptr<Factory> factory;
  CommandHooks commandHooks;
  EventHooks eventHooks;

  Plugin& operator=(Plugin&&) = delete;
  Plugin& operator=(const Plugin&) = delete;
  Plugin(Plugin&&) = delete;
  Plugin(const Plugin&) = delete;
  
protected:
  // in plugins that need for the hooks to be cleared earlier in
  // destruction, this should be called int the derived class' destructor
  void ClearHooks()
  {
    commandHooks.Clear();
    eventHooks.Clear();
  }
  
public:
  Plugin(const std::string& configName, const std::shared_ptr<Factory>& factory) :
    configName(configName),
    factory(factory)
  { }
  
  virtual ~Plugin();
  virtual void RunScript(const std::string& file) = 0;
  const std::string& Name() const { return name; }  
};

class PluginManager
{
  std::vector<std::shared_ptr<Plugin>> plugins;
  
  static Plugin& DerefSharedPtr(const std::shared_ptr<Plugin>& plugin)
  {
    assert(plugin);
    return *plugin;
  }
  
public:
  typedef decltype(boost::make_transform_iterator(plugins.begin(), DerefSharedPtr)) iterator;

  PluginManager(std::vector<std::shared_ptr<Plugin>>&& plgins) :
    plugins(plugins)
  { }
  
  iterator begin()
  {
    return boost::make_transform_iterator(plugins.begin(), DerefSharedPtr);
  }
  
  iterator end()
  {
    return boost::make_transform_iterator(plugins.end(), DerefSharedPtr);
  }
  
  void TriggerEvent(Event event, ftp::Client& client, const EventHookArgs& args)
  {
    for (auto& plugin : plugins)
    {
      plugin->Trigger(event, client, args);
    }
  }

  void TriggerEvent(Event event, ftp::Client& client)
  {
    Trigger(event, client, EventHookArgs());
  }
  
  CommandDefOpt LookupCommand(const std::string& command)
  {
    for (auto& plugin : plugins)
    {
      auto def = plugin->LookupCommand(command);
      if (def) return def;
    }
    return boost::none;
  }
};

class PluginState
{
	__thread Plugin* currentPlugin;
	
public:
	static void SwapInPlugin(Plugin& plugin)
	{
		currentPlugin = &plugin;
	}
	
	static void SwapOutPlugin()
	{
		currentPlugin = nullptr;
	}
	
	Plugin& Current()
	{
		assert(currentPlugin);
		return *currentPlugin;
	}
};


class ScopeSwapPlugin
{
public:
	ScopeSwapPlugin(Plugin& plugin)
	{
		PluginState::SwapInPlugin(plugin);
	}
	
	~ScopeSwapPlugin()
	{
		PluginState::SwapOutPlugin();
	}
};

struct LockGuard
{
	LockGuard()
	{
		PluginState::Current().Lock();
	}
	
	~LockGuard()
	{
		PluginState::Current().Unlock();
	}
};

struct UnlockGuard
{
  UnlockGuard()
  {
		PluginState::Current().Unlock();
  }
  
  ~UnlockGuard()
  {
		PluginState::Current().Lock();
  }
};

} /* plugin namespace */
