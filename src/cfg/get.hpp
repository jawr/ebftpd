#ifndef __CFG_GET_HPP
#define __CFG_GET_HPP

#include <memory>
#include <functional>
#include "cfg/config.hpp"

namespace cfg
{

void UpdateShared(const std::shared_ptr<Config> newShared);
void UpdateLocal();
const Config& Get();
bool RequireStopStart();
void ConnectUpdatedSlot(const std::function<void()>& slot);

} /* cfg namespace */

#endif
