#ifndef __PLUGIN_ERROR_HPP
#define __PLUGIN_ERROR_HPP

#include "util/error.hpp"

namespace plugin
{

struct PluginError : public util::RuntimeError
{
public:
  PluginError() : std::runtime_error("Unknown script error") { }
	PluginError(const std::string& message) : std::runtime_error(message) { }
};

struct InitialiseError : public PluginError
{
public:
  InitialiseError(const std::string& message) : std::runtime_error(message) { }
};

struct ValueError : public PluginError
{
public:
  ValueError() : std::runtime_error("Value error") { }
  ValueError(const std::string& message) : std::runtime_error(message) { }
};

struct NotConstructable : public PluginError
{
  NotConstructable() : std::runtime_error("Not constructible") { }
};

} /* script namespace */

#endif
