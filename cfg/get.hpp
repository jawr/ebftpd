#ifndef __CFG_GET_HPP
#define __CFG_GET_HPP

namespace cfg
{

class Config;

void UpdateShared(const std::tr1::shared_ptr<Config> newShared);
Config const* Get(bool update = false);

} /* cfg namespace */

#endif
