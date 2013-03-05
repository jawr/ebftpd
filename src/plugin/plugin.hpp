#ifndef __PLUGIN_PLUGIN_HPP
#define __PLUGIN_PLUGIN_HPP

#include <string>
#include "util/error.hpp"

namespace plugin
{

struct Plugin
{
  const std::string& Name() = 0;
  const std::string& Version() = 0;
	util::Error Load() = 0;
  util::Error Unload() = 0;
};

} /* script namespace */

#endif
