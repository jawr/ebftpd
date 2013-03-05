#ifndef __PLUGIN_ERROR_HPP
#define __PLUGIN_ERROR_HPP

#include "util/error.hpp"

namespace plugin
{

struct ScriptError : public util::RuntimeError
{
public:
  ScriptError() : std::runtime_error("Unknown script error") { }
	ScriptError(const std::string& message) : std::runtime_error(message) { }
};

struct ValueError : public ScriptError
{
public:
  ValueError() : std::runtime_error("Value error") { }
  ValueError(const std::string& message) : std::runtime_error(message) { }
};

struct NotConstructable : public ScriptError
{
  NotConstructable() : std::runtime_error("Not constructible") { }
};

} /* script namespace */

#endif
