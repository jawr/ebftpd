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

#ifndef __DB_STATS_SERIALIZATION_HPP
#define __DB_STATS_SERIALIZATION_HPP

namespace mongo
{
class BSONObj;
}

namespace stats
{
enum class Timeframe : unsigned;
class Stat;
}

namespace db { namespace stats
{

mongo::BSONObj Serialize(::stats::Timeframe timeframe);
::stats::Stat Unserialize(const mongo::BSONObj& obj);

} /* stats namespace */
} /* db namespace */

#endif
