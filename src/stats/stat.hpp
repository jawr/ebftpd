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

#ifndef __STATS_DAY_HPP
#define __STATS_DAY_HPP

#include <cstdint>
#include "stats/types.hpp"
#include "acl/types.hpp"

namespace db { namespace bson
{
struct Stat;
} 
}

namespace stats
{

class Stat
{
  int32_t id;

  int files;
  long long kBytes;
  long long xfertime;
  mutable double speed;
  
public:
  Stat() : id(-1), files(0), kBytes(0), xfertime(0), speed(-1) { }  
  Stat(int32_t id) : id(id), files(0), kBytes(0), xfertime(0), speed(-1) { }

  Stat(int32_t id, int files, long long kBytes, long long xfertime) :
    id(id), files(files), kBytes(kBytes), xfertime(xfertime), speed(-1)
  { } 

  Stat(int32_t id, const Stat& stat) :
    id(id), files(stat.files), kBytes(stat.kBytes), 
    xfertime(stat.xfertime), speed(-1)
  { }

  int32_t ID() const { return id; }
  int Files() const { return files; }
  long long KBytes() const { return kBytes; }
  long long Xfertime() const { return xfertime; }
  
  double Speed() const
  {
    if (speed < 0)
    {
      if (xfertime == 0) speed = kBytes;
      else speed = kBytes / (xfertime / 1000.0);
    }
    return speed;
  }

  void Incr(const Stat& stat)
  {
    files += stat.files;
    kBytes += stat.kBytes;
    xfertime += stat.xfertime;
  }
  
  friend struct db::bson::Stat;
};

// end
}
#endif
