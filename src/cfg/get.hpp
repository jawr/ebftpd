#ifndef __CFG_GET_HPP
#define __CFG_GET_HPP

#include "cfg/config.hpp"
#include <memory>

namespace cfg
{

void UpdateShared(const std::shared_ptr<Config> newShared);
const Config& Get(bool update = false);

} /* cfg namespace */

#endif
