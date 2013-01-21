#ifndef __CFG_GET_HPP
#define __CFG_GET_HPP

#include <memory>
#include "cfg/config.hpp"

namespace cfg
{

void UpdateShared(const std::shared_ptr<Config> newShared);
void UpdateLocal();
const Config& Get();
bool RequireStopStart();

} /* cfg namespace */

#endif
