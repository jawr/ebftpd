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

#ifndef __DB_STATS_PROTOCOL_HPP
#define __DB_STATS_PROTOCOL_HPP

#include "acl/types.hpp"

namespace stats
{
enum class Timeframe : unsigned;
}

namespace db { namespace stats
{

class Traffic;

void ProtocolUpdate(acl::UserID uid, long long sendBytes, long long receiveBytes);
Traffic ProtocolUser(acl::UserID uid, ::stats::Timeframe timeframe);
Traffic ProtocolTotal(::stats::Timeframe timeframe);

} /* stats namespace */
} /* db namespace */

#endif
