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

