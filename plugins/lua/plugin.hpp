#ifndef __LUA_PLUGIN_HPP
#define __LUA_PLUGIN_HPP

#include "plugin/plugin.hpp"

struct lua_State;

namespace lua
{

class Factory;

class Plugin : public plugin::Plugin
{
  lua_State* L;

  void Initialise();
  void Cleanup();
  
public:
  Plugin(const std::shared_ptr<plugin::Factory>& factory) :
    plugin::Plugin(factory),
    L(nullptr)
  {
    Initialise();
  }
  
  ~Plugin() { Cleanup(); }
  void RunScript(const std::string& file);
};

class Factory : public plugin::Factory
{
  constexpr static const char* name = "Lua";
  constexpr static const char* version = "0.1";
  
  void Initialise();
  
public:
  const char* Name() const { return name; }
  const char* Version() const { return version; }
  Plugin* Create();
};

} /* lua namespace */

#endif
