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

#include "plugin/plugin.hpp"
#include "version.hpp"

struct Plugin : plugin::Plugin
{
  Plugin(const plugin::PluginDriver& driver) :
    plugin::Plugin(driver)
  { }
};

struct PluginFactory : plugin::PluginFactory
{
  const char* Name() const { return "test"; }
  const char* Description() const { return "test"; }
  const char* PluginVersion() const { return "0.1"; }
  const char* ServerVersion() const { return EBFTPD_VERSION; }
  const char* Author() const { return "ebftpd team"; }
  plugin::Plugin* Create(const plugin::PluginDriver& driver) const
  {
    return new Plugin(driver);
  }
};

extern "C" PluginFactory* CreateFactory()
{
  return new PluginFactory();
}

