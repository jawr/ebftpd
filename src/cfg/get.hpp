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
void StopStartCheck();
void ConnectUpdatedSlot(const std::function<void()>& slot);

} /* cfg namespace */

#endif
