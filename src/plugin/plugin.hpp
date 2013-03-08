#ifndef __PLUGIN_PLUGIN_HPP
#define __PLUGIN_PLUGIN_HPP

#include <vector>
#include <memory>
#include <cassert>
#include <utility>
#include <dlfcn.h>
#include <boost/thread/once.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/optional/optional_fwd.hpp>
#include <boost/variant/variant_fwd.hpp>

namespace ftp
{
class Client;
}

namespace plugin
{

class PluginFactory;

class PluginDriver
{
	struct Data
	{
		Data& operator=(const Data&) = delete;
		Data(const Data&) = delete;
	
		PluginFactory* factory;
		void* handle;
		
		Data() : factory(nullptr), handle(nullptr) { }
		~Data();
	};
  
  std::shared_ptr<Data> data;
	
public:
  PluginDriver(const std::string& name);
  
  PluginFactory& operator*() { return *data->factory; }
  const PluginFactory& operator*() const { return *data->factory; }
  
  PluginFactory* operator->() { return data->factory; }
  const PluginFactory* operator->() const { return data->factory; }
  
  static PluginDriver Load(const std::string& path);
};

class CommandHooks;
class CommandHook;
class EventHooks;

class Plugin
{
  Plugin operator=(const Plugin&) = delete;
  Plugin operator=(Plugin&&) = delete;
  Plugin(const Plugin&) = delete;
  Plugin(Plugin&&) = delete;
  
  PluginDriver driver;                // referenced counted copy of driver to prevent desrtuction before plugin 

protected:
  std::unique_ptr< ::plugin::CommandHooks> commandHooks;
  std::unique_ptr< ::plugin::EventHooks> eventHooks;

  void LoadScripts();                                 // script engine plugins must call this method as their last task
                                                      // during construction, this will load any scripts defined in the config
  virtual void LoadScript(const std::string& /* path */) { }  // must be overrode for script engine plugins
  
public:
  Plugin(const PluginDriver& driver);
  virtual ~Plugin();
  
  const char* Name() const;
  virtual void Lock() { }             // locking mechanisms for scripting languages that require locking around
  virtual void Unlock() { }           // the interperter (python)
  
  ::plugin::CommandHooks& CommandHooks() { return *commandHooks; }
  const ::plugin::CommandHooks& CommandHooks() const { return *commandHooks; }
  
  ::plugin::EventHooks& EventHooks() { return *eventHooks; }
  const ::plugin::EventHooks& EventHooks() const { return *eventHooks; }
};

struct PluginFactory
{
  PluginFactory& operator=(const PluginFactory&) = delete;
  PluginFactory& operator=(PluginFactory&&) = delete;
  PluginFactory(const PluginFactory&) = delete;
  PluginFactory(PluginFactory&&) = delete;

  PluginFactory() = default;
  
  virtual ~PluginFactory() { }
	virtual const char* Name() const = 0;                   // single word name for plugin, should match shared object name 'name.so'
	virtual const char* Description() const = 0;            // full description / multiple word name
	virtual const char* PluginVersion() const = 0;          // version of the plugin
  virtual const char* ServerVersion() const = 0;          // version of ebftpd built against
	virtual const char* Author() const = 0;                 // plugin author
	virtual Plugin* Create(const PluginDriver& driver) const = 0;  // factory method
};

class PluginManager;

typedef boost::variant<std::string, long long, double, int> EventHookArg;
typedef std::vector<EventHookArg> EventHookArgs;
enum class Event;

class PluginCollection
{
  struct Data
  {
    Data& operator=(const Data&) = delete;
    Data(const Data&) = delete;
    
    std::vector<Plugin*> plugins;
    
    Data() = default;
    
    ~Data()
    {
      for (auto plugin : plugins)
        delete plugin;
    }
  };
  
  std::shared_ptr<Data> data;
  
  PluginCollection() :
    data(new Data())
  { }
    
public:
  boost::optional<std::pair<CommandHook, Plugin*>> LookupCommand(const std::string& command);
  bool TriggerEvent(Event event, ftp::Client& client, const EventHookArgs& args);
  bool TriggerEvent(Event event, ftp::Client& client);

  friend class PluginManager;
};

class PluginManager
{
  mutable boost::shared_mutex mutex;
  std::vector<PluginDriver> drivers;

  static boost::once_flag instanceOnce;
  static std::unique_ptr<PluginManager> instance;
  
  PluginManager& operator=(const PluginManager&) = delete;
  PluginManager& operator=(PluginManager&&) = delete;
  PluginManager(const PluginManager&) = delete;
  PluginManager(PluginManager&&) = delete;

  PluginManager() = default;
  
  void LoadDriver(const std::string& name);
  void UnloadDriver(const std::string& name);
  
  static void CreateInstance()
  {
    instance.reset(new PluginManager());
  }
  
  void UnloadAllNoLock();
  
public:
  ~PluginManager();

  PluginCollection CreatePlugins() const;
  void Reload();
  void UnloadAll();
  
  static PluginManager& Get()
  {
    boost::call_once(&CreateInstance, instanceOnce);
    return *instance;
  }
};

class PluginState
{
	static __thread Plugin* currentPlugin;

  PluginState& operator=(const PluginState&) = delete;
  PluginState& operator=(PluginState&&) = delete;
	PluginState(const PluginState&) = delete;
  PluginState(PluginState&&) = delete;
  
  PluginState() = default;
  
public:
	static void SwapInPlugin(Plugin& plugin)
	{
		currentPlugin = &plugin;
	}
	
	static void SwapOutPlugin()
	{
		currentPlugin = nullptr;
	}
	
	static Plugin& Current()
	{
		assert(currentPlugin);
		return *currentPlugin;
	}
};


class ScopeSwapPlugin
{
  ScopeSwapPlugin& operator=(const ScopeSwapPlugin&) = delete;
  ScopeSwapPlugin& operator=(ScopeSwapPlugin&&) = delete;
  ScopeSwapPlugin(const ScopeSwapPlugin&) = delete;
  ScopeSwapPlugin(ScopeSwapPlugin&&) = delete;
  
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
  LockGuard& operator=(const LockGuard&) = delete;
  LockGuard& operator=(LockGuard&&) = delete;
  LockGuard(const LockGuard&) = delete;
  LockGuard(LockGuard&&) = delete;

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
  UnlockGuard& operator=(const UnlockGuard&) = delete;
  UnlockGuard& operator=(UnlockGuard&&) = delete;
  UnlockGuard(const UnlockGuard&) = delete;
  UnlockGuard(UnlockGuard&&) = delete;

  UnlockGuard()
  {
		PluginState::Current().Unlock();
  }
  
  ~UnlockGuard()
  {
		PluginState::Current().Lock();
  }
};

void Initialise();

} /* plugin namespace */

#endif
