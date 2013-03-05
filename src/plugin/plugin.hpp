#ifndef __PLUGIN_FACTORY_HPP
#define __PLUGIN_FACTORY_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <boost/thread/mutex.hpp>

#include <iostream>

namespace plugin
{

class Factory;

// either of these classes should throw InitialiseError on construction failure

// client specific plugin instance
class Plugin
{
  // store an instance of factory to ensure it is never
  // destructed before the plugins
  std::shared_ptr<Factory> factory;

  Plugin& operator=(Plugin&&) = delete;
  Plugin& operator=(const Plugin&) = delete;
  Plugin(Plugin&&) = delete;
  Plugin(const Plugin&) = delete;
  
public:
  Plugin(const std::shared_ptr<Factory>& factory) : factory(factory) { }
  virtual ~Plugin() { }
  virtual void RunScript(const std::string& path) = 0;
};

// global plugin instance for creating client specific instances
class Factory : public std::enable_shared_from_this<Factory>
{
  Factory& operator=(Factory&&) = delete;
  Factory& operator=(const Factory&) = delete;
  Factory(Factory&&) = delete;
  Factory(const Factory&) = delete;
  
public:
  Factory() = default;
  virtual ~Factory() { }
  virtual const char* Name() const = 0;        // name of plugin
  virtual const char* Version() const = 0;     // version of plugin
  virtual Plugin* Create() = 0; // create a plugin instance for a client
};

class FactoryHolder
{
  std::string path;
  std::shared_ptr<void> library;
  std::shared_ptr< ::plugin::Factory> factory;
  
public:
  FactoryHolder(const std::string& path, 
                const std::shared_ptr<void>& library,
                const std::shared_ptr<Factory>& factory) :
    path(path),
    library(library),
    factory(factory)
  { }
  
  const std::string& Path() const { return path; }
  ::plugin::Factory& Factory() { return *factory; }
  const ::plugin::Factory& Factory() const { return *factory; }
};

class FactoryManager
{
  boost::mutex mutex;
  std::unordered_map<std::string, FactoryHolder> factories;
  
  static std::unique_ptr<FactoryManager> instance;
  
public:
  // create and return all the plugins currently registered for
  // a new client connection
  std::vector<std::shared_ptr<Plugin>> CreatePlugins()
  {
    std::vector<std::shared_ptr<Plugin>> plugins;
    boost::lock_guard<boost::mutex> lock(mutex);
    for (auto& kv : factories)
    {
      plugins.emplace_back(kv.second.Factory().Create());
    }
    return plugins;
  }

  void Rehash();
  void Cleanup();
  
  static FactoryManager& Get()
  {
    if (!instance.get()) instance.reset(new FactoryManager());
    return *instance;
  }
};

FactoryHolder CreateFactory(const std::string& path);
void InitialiseFactories();

} /* plugin namespace */

#endif
