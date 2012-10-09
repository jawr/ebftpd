#ifndef __CFG_GET_HPP
#define __CFG_GET_HPP

#include "cfg/config.hpp"
#include <tr1/memory>

namespace cfg
{

void UpdateShared(const std::shared_ptr<Config> newShared);
const Config& Get(bool update = false);

} /* cfg namespace */

#endif
