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
