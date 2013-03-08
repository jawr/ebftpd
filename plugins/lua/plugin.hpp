#ifndef __LUA_PLUGIN_HPP
#define __LUA_PLUGIN_HPP

#include "plugin/plugin.hpp"
#include "version.hpp"

struct lua_State;

namespace lua
{

class Factory;

class Plugin : public plugin::Plugin
{
  lua_State* L;

  void Initialise();
  void Cleanup();
  void LoadScript(const std::string& path);
  
public:
  Plugin(const plugin::PluginDriver& driver) :
    plugin::Plugin(driver),
    L(nullptr)
  {
    Initialise();
  }
  
  ~Plugin() { Cleanup(); }
};

class Factory : public plugin::PluginFactory
{
  constexpr static const char* name = "Lua";
  constexpr static const char* description = "Lua scripting";
  constexpr static const char* pluginVersion = "0.1";
  constexpr static const char* serverVersion = EBFTPD_VERSION;
  constexpr static const char* author = "ebftpd team";
  
  void Initialise();
  
public:
  const char* Name() const { return name; }
  const char* Description() const { return description; }
  const char* PluginVersion() const { return pluginVersion; }
  const char* ServerVersion() const { return serverVersion; }
  const char* Author() const { return author; }
  Plugin* Create(const plugin::PluginDriver& driver) const;
};

} /* lua namespace */

#endif
